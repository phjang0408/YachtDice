#include "Scorer.h"

std::array<ScoreSlot, static_cast<size_t>(ScoreCategory::COUNT)>
Scorer::MakePreviewScores(std::array<int,5>& dice) {
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

std::array<int, 6> Scorer::makeFreqArray(const std::array<int,5>& diceVec) {
	std::array<int, 6> freq = { 0 };

	// (diceVec의 값 - 1) = freq의 index (1~6 에서 0~5로)
	for (auto it : diceVec) {
		freq[it - 1]++;
	}
	return freq;
}

// category에 따라서, 계산 방법을 다르게 
int Scorer::CalcByCategory(ScoreCategory category, 	const std::array<int,5>&dice,
	const std::array<int, 6>& freq) {
	switch (category) {
	case(ScoreCategory::Ones):
	case(ScoreCategory::Twos):
	case(ScoreCategory::Threes):
	case(ScoreCategory::Fours):
	case(ScoreCategory::Fives):
	case(ScoreCategory::Sixes):
		return Scorer::Calc_Upper(category, freq);
	case(ScoreCategory::Choice):
		return Scorer::Calc_Choice(dice);
	case(ScoreCategory::FourOfKind):
		return Scorer::Calc_FourOfKind(freq);
	case(ScoreCategory::FullHouse):
		return Scorer::Calc_FullHouse(freq);
	case(ScoreCategory::SmallStraight):
		return Scorer::Calc_SmallStraight(freq);
	case(ScoreCategory::LargeStraight):
		return Scorer::Calc_LargeStraight(freq);
	case(ScoreCategory::Yacht):
		return Scorer::Calc_Yacht(freq);
	}
}

int Scorer::Calc_Upper(ScoreCategory category, const std::array<int, 6> freq) {
	int CalcUp_Score = static_cast<int>(category);	// 0번부터 시작하니까
	return freq[CalcUp_Score] * (CalcUp_Score + 1);
}

int Scorer::Calc_Choice(const std::array<int,5>& dice) {
	int Choice_sum = 0;
	for (auto it = 0; it < dice.size(); it++) {
		Choice_sum += dice[it];
	}
	return Choice_sum;
}
int Scorer::Calc_FourOfKind( const std::array<int, 6>& freq) {
	int FourOfKind_Score = 0;
	for (int i = 0; i < freq.size(); i++) {
		if (freq[i] == 4) {
			FourOfKind_Score = 4 * (i + 1);
		}
	}
	return FourOfKind_Score;
}

int Scorer::Calc_FullHouse(const std::array<int, 6>& freq) {
	bool has_three = false, has_two = false;
	int FullHouse_Score = 0, three_idx = 0, two_idx = 0;
	for (int i = 0; i < freq.size(); i++) {
		if (freq[i] == 3) {
			has_three = true; three_idx = i;
		}
		if (freq[i] == 2) {
			has_two = true; two_idx = i;
		}
	}
	return has_three && has_two ? (three_idx + 1) * 3 + (two_idx + 1) * 2 : 0;
}
int Scorer::Calc_SmallStraight(const std::array<int, 6>& freq) {
	int is_straight = 0;
	for (int i = 0; i < freq.size(); i++) {
		if (freq[i] > 0) {
			is_straight++;
			if (is_straight == 4)	return 15;
		}
		else {
			is_straight = 0;
		}
	}
	return 0;
}
int Scorer::Calc_LargeStraight(const std::array<int, 6>& freq) {
	int is_straight = 0;
	for (int i = 0; i < freq.size(); i++) {
		if (freq[i] > 0) {
			is_straight++;
			if (is_straight == 5)	return 30;
		}
		else {
			is_straight = 0;
		}
	}
	return 0;
}
int Scorer::Calc_Yacht(const std::array<int, 6>& freq) {
	bool has_Yacht = false;
	for (int i = 0; i < freq.size(); i++) {
		if (freq[i] == 5) {
			has_Yacht = true;
		}
	}
	return has_Yacht ? 50 : 0;
}