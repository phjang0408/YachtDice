#include "DiceAnimator.h"
#include "DiceMesh.h"
#include <algorithm>
#include <cmath>

using namespace DirectX;

DiceAnimator::DiceAnimator() : rng(std::random_device{}()) {
	for (int i = 0; i < 5; ++i) {
		dice[i].position = SlotPosition(i, 0.0f);
	}
}

float DiceAnimator::Rand(float lo, float hi) {
	return std::uniform_real_distribution<float>(lo, hi)(rng);
}

XMFLOAT3 DiceAnimator::SlotPosition(int i, float keepAmount) {
	return { (i - 2) * 2.6f, 1.0f, -2.8f * keepAmount };
}

XMVECTOR DiceAnimator::FaceUpRotation(int value, float yaw) const {
	const XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	const XMVECTOR n = DieFaceNormal(value);
	const float d = XMVectorGetX(XMVector3Dot(n, up));

	XMVECTOR q;
	if (d > 0.999f) {
		q = XMQuaternionIdentity();
	}
	else if (d < -0.999f) {
		q = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), XM_PI);
	}
	else {
		// 나머지 면은 모두 90도 회전. 회전 방향 규약에 의존하지 않도록 결과를 확인해서 고른다
		const XMVECTOR axis = XMVector3Normalize(XMVector3Cross(n, up));
		q = XMQuaternionRotationNormal(axis, XM_PIDIV2);
		if (XMVectorGetX(XMVector3Dot(XMVector3Rotate(n, q), up)) < 0.5f) {
			q = XMQuaternionRotationNormal(axis, -XM_PIDIV2);
		}
	}
	// 면을 위로 돌린 뒤, 세로축으로 살짝 비틀어 자연스럽게
	return XMQuaternionMultiply(q, XMQuaternionRotationAxis(up, yaw));
}

void DiceAnimator::Hide() {
	for (auto& d : dice) {
		d.visible = false;
		d.rolling = false;
		d.keepAmount = 0.0f;
	}
}

void DiceAnimator::StartRoll(const std::array<int, 5>& values, const std::array<bool, 5>& keep) {
	for (int i = 0; i < 5; ++i) {
		DieState& d = dice[i];
		if (keep[i] && d.visible) continue;	// KEEP된 주사위는 그대로

		d.visible = true;
		d.rolling = true;
		d.time = 0.0f;
		d.delay = i * 0.05f + Rand(0.0f, 0.08f);
		d.duration = Rand(0.85f, 1.2f);
		XMStoreFloat4(&d.restRotation, FaceUpRotation(values[i], Rand(-0.3f, 0.3f)));
		XMStoreFloat3(&d.spinAxis, XMVector3Normalize(XMVectorSet(Rand(-1, 1), Rand(-1, 1), Rand(-1, 1), 0) + XMVectorSet(0.01f, 0, 0, 0)));
		d.spinAngle = XM_2PI * Rand(2.0f, 3.5f);
		d.startOffset = { Rand(-2.5f, 2.5f), 0.0f, Rand(9.0f, 12.0f) };
		d.bounceHeight = Rand(2.5f, 3.5f);
	}
}

void DiceAnimator::Update(float dt, const std::array<bool, 5>& keep) {
	for (int i = 0; i < 5; ++i) {
		DieState& d = dice[i];
		const float target = keep[i] ? 1.0f : 0.0f;
		d.keepAmount += (target - d.keepAmount) * (std::min)(1.0f, dt * 12.0f);

		const XMFLOAT3 slot = SlotPosition(i, d.keepAmount);
		if (!d.rolling) {
			d.position = slot;
			d.rotation = d.restRotation;
			continue;
		}

		d.time += dt;
		const float t = std::clamp((d.time - d.delay) / d.duration, 0.0f, 1.0f);
		const float ease = 1.0f - std::pow(1.0f - t, 3.0f);
		const float remain = 1.0f - ease;

		// 멀리서 날아와 통통 튀다가 자리에 멈춘다
		d.position.x = slot.x + d.startOffset.x * remain;
		d.position.z = slot.z + d.startOffset.z * remain;
		d.position.y = slot.y + d.bounceHeight * std::fabs(std::cos(t * XM_PI * 2.5f)) * std::pow(1.0f - t, 1.5f);

		const XMVECTOR spin = XMQuaternionRotationAxis(XMLoadFloat3(&d.spinAxis), d.spinAngle * remain);
		XMStoreFloat4(&d.rotation, XMQuaternionMultiply(XMLoadFloat4(&d.restRotation), spin));

		if (t >= 1.0f) d.rolling = false;
	}
}

bool DiceAnimator::IsRolling() const {
	return std::any_of(dice.begin(), dice.end(), [](const DieState& d) { return d.rolling; });
}

XMMATRIX DiceAnimator::GetWorld(int i) const {
	const DieState& d = dice[i];
	return XMMatrixRotationQuaternion(XMLoadFloat4(&d.rotation)) *
		XMMatrixTranslation(d.position.x, d.position.y, d.position.z);
}
