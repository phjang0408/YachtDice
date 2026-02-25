#include "Scorer.h"

std::array<ScoreSlot, static_cast<size_t>(ScoreCategory::COUNT)>
MakePreviewScores(std::vector<int>& dice) {
	// 1. 빈도수 배열 만들기
	std::array<int, 6> freq = makeFreqArray(dice);

	// 2. 예비 점수판 Prev_ScoreBoard
	std::array<ScoreSlot, static_cast<size_t>(ScoreCategory::COUNT)> Prev_ScoreBoard{};

	// 예비 점수판 채우기
	for (size_t i = 0; i < Prev_ScoreBoard.size(); i++) {
		auto category = static_cast<ScoreCategory>(i);
		Prev_ScoreBoard[i].score = CalcByCategory(category, dice, freq);
		Prev_ScoreBoard[i].used = false;
	}

	return Prev_ScoreBoard;
}

std::array<int, 6> makeFreqArray(const std::vector<int>& diceVec) {
	std::array<int, 6> freq = { 0 };

	// diceVec의값 - 1은 freq의 index (1~6 에서 0~5로)
	for (auto it : diceVec) {
		freq[it - 1]++;
	}
	return freq;
}

// category에 따라서, 계산 방법을 다르게 
int CalcByCategory(ScoreCategory category, 	const std::vector<int>&dice, 
	const std::array<int, 6>& freq) {
	switch (category) {
	case(ScoreCategory::Ones):
	case(ScoreCategory::Twos):
	case(ScoreCategory::Threes):
	case(ScoreCategory::Fours):
	case(ScoreCategory::Fives):
	case(ScoreCategory::Sixes):
		CalcUpper(category, freq);
	}

}

int CalcUpper(ScoreCategory category, const std::array<int, 6> freq) {
	int CalcUp_Score = static_cast<int>(category);	// 0번부터 시작하니까
	return freq[CalcUp_Score] * (CalcUp_Score + 1);
}