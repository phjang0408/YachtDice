#pragma once
#include "ScoreBoard.h"
// 유틸리티로 활용 할 예정이라, static으로 지정함.
class Scorer {
public:
    static std::array<ScoreSlot, static_cast<size_t>(ScoreCategory::COUNT)>
        MakePreviewScores(const std::array<int, 5>& dice);    // 점수판 만들어주는 함수

    static std::array<int, 6> makeFreqArray(const std::array<int, 5>& diceVec);

    static int CalcByCategory(ScoreCategory category,
        const std::array<int, 5>& dice,
        const std::array<int, 6>& freq);

    static int Calc_Upper(ScoreCategory category, const std::array<int, 6> freq);
    static int Calc_Choice(const std::array<int, 5>& dice);
    static int Calc_FourOfKind(const std::array<int, 6>& freq);
    static int Calc_FullHouse(const std::array<int, 6>& freq);
    static int Calc_SmallStraight(const std::array<int, 6>& freq);
    static int Calc_LargeStraight(const std::array<int, 6>& freq);
    static int Calc_Yacht(const std::array<int, 6>& freq);
};