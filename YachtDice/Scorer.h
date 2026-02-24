#pragma once
#include "ScoreCategory.h"
#include <array>

// Calc에서 모든 족보 계산한다 생각하고
// 각각의 족보를 계산하는 함수로 쪼개는 목표로
// 그래서 먼저 Calc는 빈도수배열 만들기만 해보자
int Calc(ScoreCategory category, const std::array<int,5>& diceValues) {
	// 빈도수 배열
	std::array<int, 6> count_array = { 0 };
	int Choice = 0;
	int Four_of_a_kind = 0;
	int Yacht = 0;
	for (std::array<int, 5>::const_iterator it = diceValues.begin();
		it != diceValues.end(); it++) {
		switch (*it) {
		case 1:
			count_array[0]++;	break;
		case 2:
			count_array[1]++;	break;
		case 3:
			count_array[2]++;	break;
		case 4:
			count_array[3]++;	break;
		case 5:
			count_array[4]++;	break;
		case 6:
			count_array[5]++;	break;
		}
		Choice += *it;
	}

	// 빈도수 계산
	int Aces = count_array[0] * 1;
	int Deuces = count_array[1] * 2;
	int Threes = count_array[2] * 3;
	int Fours = count_array[3] * 4;
	int Fives = count_array[4] * 5;
	int Sixes = count_array[5] * 6;

	// 빈도수 배열로 점수 계산
	int idx = 1
	for (std::array<int, 5>::const_iterator it = count_array.begin();
		it != count_array.end(); it++, idx++) {
		switch (*it) {
		case 4:
			Four_of_a_kind = *it * idx;
		case 5:
			Yacht += 50;
		}


	}
}