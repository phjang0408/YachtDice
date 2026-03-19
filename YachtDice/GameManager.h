#pragma once
#include "Dice.h"
#include "Player.h"
#include "ScoreBoard.h"
#include "Scorer.h"
#include "ConsoleUI.h"

class GameManager {
public:
	void Run();
private:
	enum class GameState { MainMenu, Playing, GameOver };
	enum class TurnState { StartTurn, Rolling, SelectScore, EndTurn };
	GameState gameState = GameState::MainMenu;
	TurnState turnState = TurnState::StartTurn;
	Dice dice;
	Player player = Player("Player1");
	ScoreBoard scoreboard;
	int currentTurn = 0;
	int rollCount = 0;
	std::array<ScoreSlot,
		static_cast<size_t>(ScoreCategory::COUNT)> preview;
	void UpdateMainMenu();
	void UpdatePlaying();
};