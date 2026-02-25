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
	int CalcUpper(ScoreCategory category, const std::array<int,6> freq);
	int CalcChoice(std::vector<int>& dice);
};

// freq array 반환 함수

// make tempscoreboard
/*
* 
int Calc(ScoreCategory category, const std::vector<int>& diceValues) {
	// 빈도수 배열
	std::array<int, 6> count_array = makeFreqArray(diceValues);

	// 빈도수 계산
	int Aces = count_array[0] * 1;
	int Deuces = count_array[1] * 2;
	int Threes = count_array[2] * 3;
	int Fours = count_array[3] * 4;
	int Fives = count_array[4] * 5;
	int Sixes = count_array[5] * 6;

	int Yacht = 0;
	int Four_of_a_kind = 0;
	for (int i : count_array) {
		if (count_array[i] == 4) {
			Four_of_a_kind = count_array[i] * (i + 1);
		}
		else if (count_array[i] == 5) {
			Yacht = count_array[i] * (i + 1);
		}
	}
	// choice
	int Choice = 0;
	for (int d : diceValues) {
		Choice += d;
	}

	
}
*/