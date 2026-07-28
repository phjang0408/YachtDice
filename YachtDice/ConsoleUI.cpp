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

void ConsoleUI::ShowHowToPlay() {
	std::cout << "\n-------------------------\n";
	std::cout << "* 한 게임은 총 12턴으로 진행됩니다." << std::endl;
	std::cout << "* 주사위 눈의 조합을 통해, 점수 카테고리를 정할 수 있습니다." << std::endl;
	std::cout << "* 한 턴에서 주사위는 최대 3번까지 굴릴 수 있습니다." << std::endl;
	std::cout << "* 원하는 주사위를 유지(keep)한 채 나머지만 다시 굴릴 수 있습니다." << std::endl;
	std::cout << "* 매 턴이 끝나면 아직 사용하지 않은 점수 카테고리 중 하나를 선택해 점수를 기록합니다." << std::endl << std::endl;
	std::cout << "* 점수 카테고리는 다음 12개입니다." << std::endl;
	std::cout << "   * 상단(나온 개수 * 해당 눈의 수): Ones, Twos, Threes, Fours, Fives, Sixes" << std::endl;
	std::cout << "   * 하단: Choice(총 합), Four of a Kind(총 합), Full House(총 합), Small Straight(15점), Large Straight(30점), Yacht(50점)" << std::endl;
	std::cout << "* 상단 카테고리 합계가 63점 이상이면 보너스 점수(35점)를 획득합니다." << std::endl;
	std::cout << "* 12턴이 모두 끝나면 게임이 종료되고 최종 점수가 집계됩니다." << std::endl;
	std::cout << "-------------------------\n";
	return;
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