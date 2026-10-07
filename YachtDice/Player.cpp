#include "Player.h"
#include <iostream>
#include <limits>

Player::Player(const std::string& name) : name(name) {}

const std::string& Player::GetName() const {
	return name;
}

std::array<bool, 5> Player::DecideKeep() const {
	std::array<bool, 5> keep{};
	while (true) {
		std::cout << "[Decide Dice's Keep! (keep = 1, non-keep = 0)] : ";	// true = 고정(다시 굴리지 않음)
		bool valid = true;
		for (int i = 0; i < 5 && valid; i++) {
			valid = static_cast<bool>(std::cin >> keep[i]);
		}
		if (valid) return keep;

		// 0/1이 아닌 입력: 입력 스트림 상태를 복구하고 다시 받는다
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << "Enter five values of 0 or 1 (e.g. 1 0 0 1 0)\n";
	}
}

ScoreCategory Player::DecideCategory(const ScoreBoard& board) const {
	int choice;
	while(true){
		std::cout << "[Rolling Chance End!]\n[Select category index] : ";
		if (!(std::cin >> choice)) {	// 숫자가 아닌 입력: 스트림 복구 (안 하면 무한 루프)
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Please enter a number (0 ~ 11)\n";
			continue;
		}
		if (choice < 0 || choice >= static_cast<int>(ScoreCategory::COUNT)) {	// 범위 밖 인덱스 차단
			std::cout << "Invalid index. Enter 0 ~ 11\n";
			continue;
		}

		auto cat = static_cast<ScoreCategory>(choice);
		if (!board.IsUsed(cat)){
			return cat;
		}
		std::cout << "Already used. Try again\n";
	}
}