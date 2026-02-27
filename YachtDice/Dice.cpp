#include "Dice.h"

void Dice::Reset() {
	for (int i = 0; i < Dice_Array.size(); i++) {
		Dice_Array[i] = 1;
		Keep_Array[i] = false;
	}
}

int Dice::Roll() {
	static std::random_device rd;	// 시드
	static std::mt19937 gen(rd());	// 난수 생성(메르센 트위스터 19937)
	static std::uniform_int_distribution<> dist(1, 6); // 범위 조정
	return dist(gen);
}

void Dice::Roll_Selected() {
	for (int i : Keep_Array) {
		if (Keep_Array[i] = false) {
			Dice_Array[i] = Dice::Roll();
		}
	}
}

void Dice::Select_Keep(const std::array<bool, 5> Request_Keep) {
	for (int i : Keep_Array) {
		Keep_Array[i] = Request_Keep[i];
	}
}

const std::array<int, 5> Dice::get_dice_values() {
	return Dice_Array;
}

const std::array<bool, 5> Dice::get_keep_status() {
	return Keep_Array;
}