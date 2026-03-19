#pragma once
#include "ScoreCategory.h"
#include <array>

// 족보당 할당 점수
struct ScoreSlot {
	bool used = false;
	int score = 0;	// 사용자가 채우는 점수칸
};

// 점수판
class ScoreBoard {
public:
	ScoreBoard();

	bool IsUsed(ScoreCategory category) const;	// slot에서 used 반환
	int GetScore(ScoreCategory category) const;	// slot에서 category에 따라, 점수 반환
	void SetScore(ScoreCategory category, int score);

	int GetSubTotalScore() const;	// 강북 지역 점수
	int GetBonus()const;			// 강북 조건 달성했는지
	int GetTotalScore() const;
private:
	// 점수판 : slots들로 구성
	std::array<ScoreSlot, static_cast<size_t>(ScoreCategory::COUNT)> slots;
};