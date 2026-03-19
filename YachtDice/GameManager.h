#pragma once
#include "Dice.h"
#include "Player.h"
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
	int currentTurn = 0;	// 현재 턴의 개념
	int rollCount = 0;		// 1턴 안에서, 몇 번의 기회가 남았는지
	std::array<ScoreSlot,
		static_cast<size_t>(ScoreCategory::COUNT)> preview; //UI용 사용자가 선택 할 주사위배열

	void UpdateMainMenu();
	void UpdatePlaying();
};