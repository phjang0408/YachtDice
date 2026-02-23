#pragma once
#include "ScoreCategory.h"
#include <array>

// 족보당 할당 점수
struct ScoreSlot {
	bool used = false;
	int score = 0;
};

// 점수판
class ScoreBoard {
public:
	ScoreBoard();

	bool IsUsed(ScoreCategory category) const;
	int GetScore(ScoreCategory category) const;
	void SetScore(ScoreCategory category, int score);
	int GetTotalScore() const;
private:
	// enum class를 정수 상수식으로 '캐스팅', 
	std::array<ScoreSlot, static_cast<size_t>(ScoreCategory::COUNT)> slots;
};