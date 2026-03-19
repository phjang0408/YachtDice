#include "GameManager.h"
#include <iostream>
void GameManager::Run() {
	// Gamestate에 따라서 진행
	while (true) {
		switch (gameState) {
		case GameState::MainMenu:
			UpdateMainMenu();
			break;
		case GameState::Playing:
			UpdatePlaying();
			break;
		case GameState::GameOver:
			std::cout << "[Game Over]\n"; // 게임 종료
			return;
		}
	}
}

void GameManager::UpdateMainMenu() {
	int choice = ConsoleUI::ShowMainMenu();
	if (choice == 1) {	// 게임 시작
		gameState = GameState::Playing;
		turnState = TurnState::StartTurn;
		currentTurn = 0;
	}
	else if (choice == 2) {	// How to Play
		std::cout << "This is for explaining game\n";
	}
	else {	// quit
		gameState = GameState::GameOver;
	}
}

void GameManager::UpdatePlaying() {
	// Turn = 12회, Roll은 Turn당 3회
	switch (turnState) 
	{
	case TurnState::StartTurn:	// Turn 시작
		dice.Reset();
		rollCount = 0;
		turnState = TurnState::Rolling;
		break;

	case TurnState::Rolling:
		std::cout << "==== Turn " << currentTurn + 1 << " / 12 =====\n";	//Turn
		ConsoleUI::ShowScoreBoard(scoreboard);	// 현재 점수판 보여주기
		dice.Roll_Selected();					// 특정 주사위만 Roll
		rollCount++;
		preview = Scorer::MakePreviewScores(dice.get_dice_values());

		ConsoleUI::ShowDice(dice.get_dice_values(), dice.get_keep_status());	// 굴린 주사위 상태 보여주기
		ConsoleUI::ShowPreview(preview);		// 카테고리 별 점수 미리보기

		if (rollCount < 3) {
			std::cout << "[Rolled : (" << rollCount << " / 3)] \n";
			std::cout << "[Roll again? (1 = yes, 0 = no)] : ";
			int cont;
			std::cin >> cont;
			if (cont == 0)
				turnState = TurnState::SelectScore;
			else {
				auto keep = player.DecideKeep();
				dice.Select_Keep(keep);
			}
		}
		else { // Roll 3번 사용 or Player가 Turn을 끝내기로 결정
			turnState = TurnState::SelectScore;
		}
		break;

	case TurnState::SelectScore: {
		ScoreCategory cat = player.DecideCategory(scoreboard);
		size_t idx = static_cast<size_t>(cat);

		scoreboard.SetScore(cat, preview[idx].score);	// 점수 입력
		turnState = TurnState::EndTurn;
		break;
	}

	case TurnState::EndTurn:
		std::cout << "[End of Turn!]\n";
		currentTurn++;
		if (currentTurn >= 12) {
			gameState = GameState::GameOver;	// 게임 종료
		}
		else {
			turnState = TurnState::StartTurn;
		}
		break;
	}
}