#pragma once
#include <array>
#include <string>
#include "ScoreBoard.h"

class Player {
public:
	Player(const std::string& name);

	const std::string& GetName() const; // 이름 출력
	ScoreBoard& GetScoreBoard();        // 점수 출력

private:
	std::string name;                   // 이름
	ScoreBoard scoreBoard;              // 점수
};
#pragma once

#include <array>

#include <string>

#include "ScoreCategory.h"

class Player {

public:

	Player(const std::string& name);

	const std::string& GetName() const;

	std::array<bool, 5> DecideKeep() const;

	ScoreCategory DecideCategory() const;

private:

	std::string name;

};