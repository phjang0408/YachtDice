#pragma once
#include <string>
#include "ScoreBoard.h"

class ConsoleUI {
public:
	static void ShowDice(const std::array<int, 5>& dice,
	const std::array<bool, 5>& keep);

	static void ShowPreview(
	const std::array<ScoreSlot,static_cast<size_t>(ScoreCategory::COUNT)>& preview
	);

	static int ShowMainMenu();

	static std::string Category_To_String(ScoreCategory category);	// UI Ãâ·Â¿ë(Enum -> string)
};