#pragma once
#include <DirectXMath.h>
#include <cstdint>
#include <vector>

// GPU 정점 레이아웃 (Renderer의 Input Layout, 셰이더 VSIn과 일치)
struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	float face;		// 주사위 면의 눈(1~6), 테이블은 0
};

// 하나의 버퍼 안에서 메시가 차지하는 구간
struct MeshRange {
	uint32_t indexCount = 0;
	uint32_t startIndex = 0;
	int32_t baseVertex = 0;
};

// 둥근 모서리 주사위(한 변 2.0)와 테이블 평면을 생성
void BuildSceneMeshes(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices,
	MeshRange& die, MeshRange& table);

// 눈 값에 해당하는 면의 로컬 법선 (+Y=1, +Z=2, +X=3, -X=4, -Z=5, -Y=6 / 마주보는 면의 합은 7)
DirectX::XMVECTOR DieFaceNormal(int value);
