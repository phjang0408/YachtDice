#pragma once
#include <array>
#include <random>

class Dice {
public:
	void Reset();
	void Roll_Selected();
	int Roll_One();
	void Select_Keep(const std::array<bool, 5> Request_Keep);

	const std::array<int, 5>& get_dice_values() const;
	const std::array<bool, 5>& get_keep_status() const;

private:
	std::array<int,5> Dice_Array;	// 주사위 값
	std::array<bool,5> Keep_Array;	// keep 상태
};