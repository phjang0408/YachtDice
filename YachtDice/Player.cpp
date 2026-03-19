#include "Player.h"
#include <iostream>

Player::Player(const std::string& name) : name(name) {}

const std::string& Player::GetName() const {
	return name;
}

std::array<bool, 5> Player::DecideKeep() const {
	std::array<bool, 5> keep{};
	std::cout << "[Decide Dice's Keep! (keep = 0, non-keep = 1)] : ";
	for (int i = 0; i < 5; i++) {
		std::cin >> keep[i];
	}
	return keep;
}

ScoreCategory Player::DecideCategory() const {
	int choice;
	std::cout << "[Rolling Chance End!]\n[Select category index] : ";
	std::cin >> choice;
	return static_cast<ScoreCategory>(choice);
}