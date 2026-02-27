#pragma once
#include "ScoreCategory.h"
#include "ScoreBoard.h"
#include <array>
#include <vector>

class Scorer {
public:
	std::array<ScoreSlot, static_cast<size_t>(ScoreCategory::COUNT)>
	MakePreviewScores(std::vector<int>& dice);

private:
	std::array<int, 6> makeFreqArray(const std::vector<int>& diceVec);
	int CalcByCategory(ScoreCategory category, const std::vector<int>& dice, const std::array<int, 6>& freq);
	int Calc_Upper(ScoreCategory category, const std::array<int,6> freq);
	int Calc_Choice(const std::vector<int>& dice);
	int Calc_FourOfKind(const std::array<int, 6>& freq);
	int Calc_FullHouse(const std::array<int, 6>& freq);
	int Calc_SmallStraight(const std::array<int, 6>& freq);
	int Calc_LargeStraight(const std::array<int, 6>& freq);
	int Calc_Yacht(const std::array<int, 6>& freq);
};