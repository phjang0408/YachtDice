#include "GameManager.h"
#include <iostream>
void GameManager::Run() {
	while (true) {
		switch (gameState) {
		case GameState::MainMenu:
			UpdateMainMenu();
			break;
		case GameState::Playing:
			UpdatePlaying();
			break;
		case GameState::GameOver:
			std::cout << "Game Over\n";
			return;
		}
	}
}

void GameManager::UpdateMainMenu() {
	int choice = ConsoleUI::ShowMainMenu();

	if (choice == 1) {
		gameState = GameState::Playing;
		turnState = TurnState::StartTurn;
		currentTurn = 0;
	}
	else if (choice == 2) {
		std::cout << "Roll dice and choose category.\n";
	}
	else {
		gameState = GameState::GameOver;
	}
}

void GameManager::UpdatePlaying() {
	switch (turnState) {
	case TurnState::StartTurn:
		dice.Reset();
		rollCount = 0;
		turnState = TurnState::Rolling;
		break;

	case TurnState::Rolling:
		dice.Roll_Selected();
		rollCount++;
		preview = Scorer::MakePreviewScores(dice.GetValues());
		ConsoleUI::ShowDice(dice.GetValues(), dice.GetKeeps());
		ConsoleUI::ShowPreview(preview);

		if (rollCount < 3) {
			auto keep = player.DecideKeep();
			dice.Select_Keep(keep);
			std::cout << "Roll again? (1=yes, 0=no): ";
			int cont;
			std::cin >> cont;
			if (cont == 0)
				turnState = TurnState::SelectScore;
		}
		else {
			turnState = TurnState::SelectScore;
		}
		break;

	case TurnState::SelectScore: {
		ScoreCategory cat = player.DecideCategory();
		size_t idx = static_cast<size_t>(cat);

		scoreboard.SetScore(cat, preview[idx].score);
		turnState = TurnState::EndTurn;
		break;
	}
	case TurnState::EndTurn:
		currentTurn++;
		if (currentTurn >= 12) {
			gameState = GameState::GameOver;
		}
		else {
			turnState = TurnState::StartTurn;
		}
		break;
	}
}