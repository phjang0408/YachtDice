#pragma once
#include <DirectXMath.h>
#include <array>
#include <random>

// 주사위 5개의 3D 위치/회전과 굴리기 애니메이션을 관리한다.
// 게임 로직(Dice)이 정한 결과 값으로 착지하도록 회전을 역산한다.
class DiceAnimator {
public:
	DiceAnimator();

	void Hide();	// 턴 시작 시: 굴리기 전까지 주사위를 숨긴다
	void StartRoll(const std::array<int, 5>& values, const std::array<bool, 5>& keep);
	void Update(float dt, const std::array<bool, 5>& keep);

	bool IsRolling() const;
	bool IsVisible(int i) const { return dice[i].visible; }
	float GetKeepAmount(int i) const { return dice[i].keepAmount; }
	DirectX::XMFLOAT3 GetPosition(int i) const { return dice[i].position; }
	DirectX::XMMATRIX GetWorld(int i) const;

	static DirectX::XMFLOAT3 SlotPosition(int i, float keepAmount);	// 주사위 자리(KEEP이면 앞쪽 줄)

private:
	struct DieState {
		bool visible = false;
		bool rolling = false;
		float time = 0.0f;
		float delay = 0.0f;
		float duration = 1.0f;
		DirectX::XMFLOAT4 restRotation{ 0, 0, 0, 1 };	// 착지 시 회전 (결과 눈이 위로)
		DirectX::XMFLOAT3 spinAxis{ 0, 1, 0 };
		float spinAngle = 0.0f;
		DirectX::XMFLOAT3 startOffset{ 0, 0, 0 };	// 던져지기 시작하는 위치 (자리 기준)
		float bounceHeight = 0.0f;
		float keepAmount = 0.0f;	// 0 ~ 1, KEEP 전환을 부드럽게
		DirectX::XMFLOAT3 position{ 0, 1, 0 };
		DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 };
	};

	float Rand(float lo, float hi);
	DirectX::XMVECTOR FaceUpRotation(int value, float yaw) const;

	std::array<DieState, 5> dice;
	std::mt19937 rng;
};
