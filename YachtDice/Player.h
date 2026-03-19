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