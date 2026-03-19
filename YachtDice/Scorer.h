#pragma once
#include "ScoreCategory.h"
#include "ScoreBoard.h"
#include <array>

class Scorer {
public:
	std::array<ScoreSlot, static_cast<size_t>(ScoreCategory::COUNT)>
	MakePreviewScores(std::array<int,5>& dice);

private:
	std::array<int, 6> makeFreqArray(const std::array<int,5>& diceVec);
	int CalcByCategory(ScoreCategory category, const std::array<int,5>& dice, const std::array<int, 6>& freq);
	int Calc_Upper(ScoreCategory category, const std::array<int,6> freq);
	int Calc_Choice(const std::array<int,5>& dice);
	int Calc_FourOfKind(const std::array<int, 6>& freq);
	int Calc_FullHouse(const std::array<int, 6>& freq);
	int Calc_SmallStraight(const std::array<int, 6>& freq);
	int Calc_LargeStraight(const std::array<int, 6>& freq);
	int Calc_Yacht(const std::array<int, 6>& freq);
};