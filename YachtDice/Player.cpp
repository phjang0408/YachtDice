#include "Player.h"
#include <iostream>

Player::Player(const std::string& name) : name(name) {}

const std::string& Player::GetName() const {
	return name;
}

std::array<bool, 5> Player::DecideKeep() const {
	std::array<bool, 5> keep{};
	std::cout << "Keep (0 or 1 for 5 dice): ";
	for (int i = 0; i < 5; i++) {
		std::cin >> keep[i];
	}
	return keep;
}

ScoreCategory Player::DecideCategory() const {
	int choice;
	std::cout << "Select category index: ";
	std::cin >> choice;
	return static_cast<ScoreCategory>(choice);
}