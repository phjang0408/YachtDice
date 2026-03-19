#pragma once
#include "ConsoleUI.h"
#include <iostream>
void ConsoleUI::ShowDice(const std::array<int, 5>& dice,
	const std::array<bool, 5>& keep) {
	std::cout << "-------------------------\n";
	std::cout << "Dice: ";
	for (int i = 0; i < 5; i++) {
		std::cout << " " << dice[i] << "  ";
	}
	std::cout << "\nKeep: ";
	for (int i = 0; i < 5; i++) {
		std::cout << "[" << (keep[i] ? "O" : "X") << "] ";
	}
	std::cout << "\n-------------------------\n";
}

int ConsoleUI::ShowMainMenu() {
	std::cout << "1. Start\n2. HowTo\n3. Quit\n";
	int input;
	std::cout << "Select Command : ";
	std::cin >> input;
	return input;
}
void ConsoleUI::ShowPreview(
	const std::array<ScoreSlot,
	static_cast<size_t>(ScoreCategory::COUNT)>& preview) {

	std::cout << "Preview Scores:\n";

	for (size_t i = 0; i < preview.size(); i++) {
		std::cout << i << ". " << Category_To_String(static_cast<ScoreCategory>(i)) 
			<< ": " << preview[i].score << "\n";

	}
	std::cout << "-------------------------\n";
}
std::string ConsoleUI::Category_To_String(ScoreCategory category) {
	switch (category) {
	case ScoreCategory::Ones: return "Ones";
	case ScoreCategory::Twos: return "Twos";
	case ScoreCategory::Threes: return "Threes";
	case ScoreCategory::Fours: return "Fours";
	case ScoreCategory::Fives: return "Fives";
	case ScoreCategory::Sixes: return "Sixes";
	case ScoreCategory::Choice: return "Choice";
	case ScoreCategory::FourOfKind: return "FourOfAKind";
	case ScoreCategory::FullHouse: return "FullHouse";
	case ScoreCategory::SmallStraight: return "SmallStraight";
	case ScoreCategory::LargeStraight: return "LargeStraight";
	case ScoreCategory::Yacht: return "Yacht";
	default: return "Unknown";
	}
}

void ConsoleUI::ShowScoreBoard(const ScoreBoard& board) {
	std::cout << "===== SCORE BOARD =====\n";
	
	for (int i = 0; i < static_cast<int>(ScoreCategory::COUNT); i++) {
		if (i == 6)	std::cout << "-------------------------\n";	// 강남-강북 구분선
		auto cat = static_cast<ScoreCategory>(i);

		std::cout << i << ". "<< Category_To_String(static_cast<ScoreCategory>(i)) << ": ";

		if (board.IsUsed(cat)) {
			std::cout << board.GetScore(cat);
		}
		else {
			std::cout << "-";
		}
		std::cout << "\n";

	}
	std::cout << "====== SubScore ======\n";
	std::cout << "Upper Sum: " << board.GetSubTotalScore() << " / 63 " << "\n";
	std::cout << "Bonus: " << board.GetBonus() << "\n";
	std::cout << "Total: " << board.GetTotalScore() << "\n";
}