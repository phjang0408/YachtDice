#include "DiceMesh.h"
#include <algorithm>

using namespace DirectX;

namespace {
	struct FaceDef {
		XMFLOAT3 n, u, v;
		int value;
	};

	const FaceDef kFaces[6] = {
		{ { 0,  1,  0 }, { 1, 0, 0 }, { 0, 0, 1 }, 1 },
		{ { 0,  0,  1 }, { 1, 0, 0 }, { 0, 1, 0 }, 2 },
		{ { 1,  0,  0 }, { 0, 0, 1 }, { 0, 1, 0 }, 3 },
		{ { -1, 0,  0 }, { 0, 0, 1 }, { 0, 1, 0 }, 4 },
		{ { 0,  0, -1 }, { 1, 0, 0 }, { 0, 1, 0 }, 5 },
		{ { 0, -1,  0 }, { 1, 0, 0 }, { 0, 0, 1 }, 6 },
	};

	constexpr float kBevel = 0.2f;	// 모서리 둥글기 반지름
	constexpr int kBevelSteps = 5;	// 모서리 분할 수
}

XMVECTOR DieFaceNormal(int value) {
	for (const auto& f : kFaces) {
		if (f.value == value) return XMLoadFloat3(&f.n);
	}
	return XMVectorSet(0, 1, 0, 0);
}

void BuildSceneMeshes(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices,
	MeshRange& die, MeshRange& table) {
	vertices.clear();
	indices.clear();

	// 면 위의 격자 좌표: 평평한 가운데는 한 칸, 둥근 모서리 부분만 촘촘하게 나눈다
	std::vector<float> coords;
	for (int k = 0; k <= kBevelSteps; ++k) coords.push_back(-1.0f + kBevel * k / kBevelSteps);
	for (int k = 0; k <= kBevelSteps; ++k) coords.push_back((1.0f - kBevel) + kBevel * k / kBevelSteps);
	const int n = static_cast<int>(coords.size());
	const float inner = 1.0f - kBevel;

	for (const auto& f : kFaces) {
		const XMVECTOR N = XMLoadFloat3(&f.n);
		const XMVECTOR U = XMLoadFloat3(&f.u);
		const XMVECTOR V = XMLoadFloat3(&f.v);
		const uint16_t base = static_cast<uint16_t>(vertices.size());

		for (int b = 0; b < n; ++b) {
			for (int a = 0; a < n; ++a) {
				const float s = coords[a], t = coords[b];
				// 정육면체 표면의 점을 안쪽 상자에 대해 반지름만큼 밀어내면 둥근 상자가 된다
				XMVECTOR p = N + U * s + V * t;
				XMVECTOR core = XMVectorClamp(p, XMVectorReplicate(-inner), XMVectorReplicate(inner));
				XMVECTOR dir = XMVector3Normalize(p - core);
				XMVECTOR pos = core + dir * kBevel;

				Vertex vtx{};
				XMStoreFloat3(&vtx.pos, pos);
				XMStoreFloat3(&vtx.normal, dir);
				vtx.uv = { (s + 1.0f) * 0.5f, (t + 1.0f) * 0.5f };
				vtx.face = static_cast<float>(f.value);
				vertices.push_back(vtx);
			}
		}
		for (int b = 0; b < n - 1; ++b) {
			for (int a = 0; a < n - 1; ++a) {
				const uint16_t i0 = static_cast<uint16_t>(base + b * n + a);
				const uint16_t i1 = i0 + 1;
				const uint16_t i2 = static_cast<uint16_t>(i0 + n);
				const uint16_t i3 = i2 + 1;
				indices.insert(indices.end(), { i0, i2, i1, i1, i2, i3 });
			}
		}
	}
	die.indexCount = static_cast<uint32_t>(indices.size());
	die.startIndex = 0;
	die.baseVertex = 0;

	// 테이블: 큰 사각형 하나
	table.startIndex = static_cast<uint32_t>(indices.size());
	table.baseVertex = static_cast<int32_t>(vertices.size());
	const float e = 40.0f;
	vertices.push_back({ { -e, 0, -e }, { 0, 1, 0 }, { 0, 0 }, 0 });
	vertices.push_back({ { -e, 0,  e }, { 0, 1, 0 }, { 0, 1 }, 0 });
	vertices.push_back({ {  e, 0,  e }, { 0, 1, 0 }, { 1, 1 }, 0 });
	vertices.push_back({ {  e, 0, -e }, { 0, 1, 0 }, { 1, 0 }, 0 });
	indices.insert(indices.end(), { 0, 1, 2, 0, 2, 3 });
	table.indexCount = 6;
}
