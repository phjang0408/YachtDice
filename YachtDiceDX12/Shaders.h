#pragma once
// 3D 장면(테이블 + 주사위)용 HLSL. 실행 시 D3DCompile로 컴파일한다.
// 주사위 눈은 텍스처 없이 픽셀 셰이더에서 UV로 직접 그린다.
static const char g_ShaderSource[] = R"(
cbuffer FrameCB : register(b0)
{
    float4x4 ViewProj;
    float4   EyePos;
    float4   LightDir;     // xyz: direction toward the light
    float4   DicePos[5];   // xyz: die center, w: 1 if visible (used for table contact shadows)
};

cbuffer ObjectCB : register(b1)
{
    float4x4 World;
    float4   BaseColor;
    float4   Params;       // x: 0 = table, 1 = die / y: hover highlight / z: keep amount
};

struct VSIn
{
    float3 pos  : POSITION;
    float3 nrm  : NORMAL;
    float2 uv   : TEXCOORD0;
    float  face : TEXCOORD1;
};

struct PSIn
{
    float4 pos  : SV_POSITION;
    float3 wpos : TEXCOORD0;
    float3 nrm  : TEXCOORD1;
    float2 uv   : TEXCOORD2;
    nointerpolation float face : TEXCOORD3;
};

PSIn VSMain(VSIn v)
{
    PSIn o;
    float4 w = mul(float4(v.pos, 1.0), World);
    o.wpos = w.xyz;
    o.pos  = mul(w, ViewProj);
    o.nrm  = mul(v.nrm, (float3x3)World);
    o.uv   = v.uv;
    o.face = v.face;
    return o;
}

float Hash(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}

float Noise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    float2 u = f * f * (3.0 - 2.0 * f);
    float a = Hash(i);
    float b = Hash(i + float2(1, 0));
    float c = Hash(i + float2(0, 1));
    float d = Hash(i + float2(1, 1));
    return lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
}

// Pips on a 3x3 grid, bit index = row * 3 + col
static const uint PipMask[7] = { 0u, 0x010u, 0x101u, 0x111u, 0x145u, 0x155u, 0x16Du };

float PipDistance(float2 uv, int value)
{
    uint mask = PipMask[clamp(value, 0, 6)];
    float d = 10.0;
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        if (((mask >> i) & 1u) != 0u)
        {
            float2 c = float2(0.25 + 0.25 * (i % 3), 0.25 + 0.25 * (i / 3));
            d = min(d, distance(uv, c));
        }
    }
    return d;
}

float4 PSMain(PSIn i) : SV_TARGET
{
    float3 N = normalize(i.nrm);
    float3 L = normalize(LightDir.xyz);
    float3 V = normalize(EyePos.xyz - i.wpos);
    float3 H = normalize(L + V);

    // derivatives must be taken outside of branches
    int   value = (int)round(i.face);
    float pipD  = PipDistance(i.uv, value);
    float pipAA = fwidth(pipD) + 1e-4;

    float3 albedo = pow(BaseColor.rgb, 2.2);
    float3 emissive = 0;
    float  shadow = 0;
    float  specStrength;
    float  specPower;

    if (Params.x < 0.5)
    {
        // felt table
        float n = Noise(i.wpos.xz * 4.0) * 0.6 + Noise(i.wpos.xz * 17.0) * 0.4;
        albedo *= 0.82 + 0.36 * n;
        float2 vig = (i.wpos.xz - float2(0.0, -1.0)) * float2(0.055, 0.085);
        albedo *= saturate(1.2 - dot(vig, vig));

        // soft contact shadows cast along the light direction
        [unroll]
        for (int k = 0; k < 5; ++k)
        {
            if (DicePos[k].w > 0.5)
            {
                float3 p = DicePos[k].xyz;
                float  h = max(p.y - 1.0, 0.0);
                float2 c = p.xz - L.xz * (p.y / max(L.y, 0.2));
                float  radius = 1.45 + h * 0.3;
                float  s = 1.0 - smoothstep(radius * 0.35, radius, length(i.wpos.xz - c));
                shadow = max(shadow, s * saturate(1.0 - h * 0.15));
            }
        }
        specStrength = 0.02;
        specPower = 8.0;
    }
    else
    {
        // die body + pips
        float radius = (value == 1) ? 0.14 : 0.085;
        float pip  = 1.0 - smoothstep(radius - pipAA, radius + pipAA, pipD);
        float ring = (1.0 - smoothstep(radius, radius + 0.035, pipD)) * (1.0 - pip);
        float3 pipColor = (value == 1) ? float3(0.60, 0.02, 0.03) : float3(0.015, 0.015, 0.02);

        albedo = lerp(albedo, float3(1.0, 0.72, 0.25), Params.z * 0.45);
        albedo *= 1.0 - ring * 0.25;
        albedo = lerp(albedo, pipColor, pip);

        specStrength = lerp(0.55, 0.25, pip);
        specPower = 90.0;
        emissive = Params.y * float3(0.10, 0.09, 0.06) + Params.z * float3(0.05, 0.035, 0.0);
    }

    float  ndl     = saturate(dot(N, L)) * (1.0 - 0.6 * shadow);
    float3 ambient = lerp(float3(0.05, 0.055, 0.07), float3(0.22, 0.23, 0.26), N.y * 0.5 + 0.5) * (1.0 - 0.45 * shadow);
    float  spec    = specStrength * pow(saturate(dot(N, H)), specPower) * ndl;
    float  rim     = pow(1.0 - saturate(dot(N, V)), 4.0) * 0.08 * Params.x;

    float3 color = albedo * (ambient + ndl * float3(1.0, 0.96, 0.9) * 1.1) + spec + rim + emissive;
    color = color / (1.0 + color * 0.25) * 1.15;
    return float4(pow(saturate(color), 1.0 / 2.2), 1.0);
}
)";
