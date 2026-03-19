#include "ScoreBoard.h"

// 생성자
ScoreBoard::ScoreBoard() {
    for (auto& slot : slots) {
        slot.used = false;
        slot.score = 0;
    }
}
//getter
bool ScoreBoard::IsUsed(ScoreCategory category) const {
    return slots[static_cast<size_t>(category)].used;
}

int ScoreBoard::GetScore(ScoreCategory category) const {
    return slots[static_cast<size_t>(category)].score;
}

void ScoreBoard::SetScore(ScoreCategory category, int score) {
    auto& slot = slots[static_cast<size_t>(category)];
    slot.used = true;
    slot.score = score;
}

int ScoreBoard::GetSubTotalScore() const {
    int sum = 0;
    for (int i = static_cast<int>(ScoreCategory::Ones);
        i <= static_cast<int>(ScoreCategory::Sixes); ++i) {

        auto each_numbers_under_six = static_cast<ScoreCategory>(i);    // enum으로 변환

        if (IsUsed(each_numbers_under_six)) {
            sum += GetScore(each_numbers_under_six);
        }
    }
    return sum;
}

int ScoreBoard::GetBonus() const {
    return GetSubTotalScore() >= 63 ? 35 : 0;
}

// 총점 계산
int ScoreBoard::GetTotalScore() const {
    int total = 0;
    for (const auto& slot : slots) {
        if (slot.used)
            total += slot.score;
    }
    return total;
}