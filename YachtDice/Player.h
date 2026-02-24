#pragma once
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