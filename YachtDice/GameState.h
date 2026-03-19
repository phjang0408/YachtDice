#pragma once
// 상위 상태
enum class GameState {
	MainMenu,
	Playing,	// TurnState 소유
	GameOver
};
// 하위 상태
enum class TurnState {
	StartTurn,
	Rolling,
	SelectKeep,
	EndTurn
};