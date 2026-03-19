#pragma once
#include "ConsoleUI.h"
#include <iostream>
void ConsoleUI::ShowDice(const std::array<int, 5>& dice,

	const std::array<bool, 5>& keep) {

	std::cout << "Dice: ";

	for (int i = 0; i < 5; i++) {

		std::cout << dice[i] << "(" << keep[i] << ") ";

	}

	std::cout << "\n";

}

void ConsoleUI::ShowPreview(

	const std::array<ScoreSlot,
	static_cast<size_t>(ScoreCategory::COUNT)>& preview) {

	std::cout << "Preview Scores:\n";

	for (size_t i = 0; i < preview.size(); i++) {

		std::cout << i << ": " << preview[i].score << "\n";

	}

}

int ConsoleUI::ShowMainMenu() {

	std::cout << "1. Start\n2. HowTo\n3. Quit\n";

	int input;

	std::cin >> input;

	return input;

}