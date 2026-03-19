#include "Dice.h"

void Dice::Reset() {
	for (size_t i = 0; i < Dice_Array.size(); i++) {
		Dice_Array[i] = 1;
		Keep_Array[i] = false;
	}
}

int Dice::Roll_One() {
	static std::random_device rd;	// 시드
	static std::mt19937 gen(rd());	// 난수 생성(메르센 트위스터 19937)
	static std::uniform_int_distribution<> dist(1, 6); // 범위 조정
	return dist(gen);
}

void Dice::Roll_Selected() {
	for (size_t i = 0; i < Dice_Array.size(); i++) {
		if (!Keep_Array[i]) {
			Dice_Array[i] = Dice::Roll_One();
		}
	}
}

void Dice::Select_Keep(const std::array<bool, 5> Request_Keep) {
	for (size_t i = 0; i < Dice_Array.size(); i++) {
		Keep_Array[i] = Request_Keep[i];
	}
}

const std::array<int, 5>& Dice::get_dice_values() const{
	return Dice_Array;
}

const std::array<bool, 5>& Dice::get_keep_status() const{
	return Keep_Array;
}