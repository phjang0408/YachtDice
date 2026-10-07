#pragma once
#include "Renderer.h"
#include "Ui.h"
#include "DiceAnimator.h"
#include "Dice.h"
#include "ScoreBoard.h"
#include "Scorer.h"
#include <array>

// 콘솔판 GameManager를 창 기반(이벤트 + 매 프레임 갱신) 구조로 옮긴 클래스.
// 게임 규칙은 기존 Dice / Scorer / ScoreBoard를 그대로 사용한다.
class VisualGame {
public:
	void Initialize(Renderer* renderer);
	void Update(float dt);
	void Render();

	void OnMouseMove(int x, int y);
	void OnMouseDown(int x, int y);
	void OnKeyDown(WPARAM key);

	bool IsHoveringClickable() const { return hover.type != HitType::None; }
	bool WantsQuit() const { return quitRequested; }

private:
	static constexpr int kCategoryCount = static_cast<int>(ScoreCategory::COUNT);
	static constexpr int kMaxTurns = 12;
	static constexpr int kMaxRolls = 3;

	enum class Screen { MainMenu, HowTo, Playing, GameOver };
	enum class TurnState { WaitRoll, Rolling, Choosing };
	enum class HitType { None, Start, HowTo, Quit, Back, Roll, Die, Category, PlayAgain, ToMenu };
	struct Hit {
		HitType type = HitType::None;
		int index = -1;
	};

	// 게임 흐름
	void StartNewGame();
	void StartTurn();
	void RollDice();
	void ToggleKeep(int index);
	void ChooseCategory(int index);
	bool CanToggleKeep() const;
	bool CanRoll() const;

	// 화면
	void UpdateLayout();
	void UpdateCamera();
	void UpdateDiceTransforms();
	Hit HitTest(DirectX::XMFLOAT2 p) const;
	DirectX::XMFLOAT2 ToVirtual(int x, int y) const;

	void DrawMainMenu();
	void DrawHowTo();
	void DrawPlaying();
	void DrawGameOver();
	void DrawScoreBoard();
	bool IsHovered(HitType type, int index = -1) const;

	Renderer* renderer = nullptr;
	Ui ui;

	Screen screen = Screen::MainMenu;
	TurnState turnState = TurnState::WaitRoll;
	Dice dice;
	ScoreBoard scoreboard;
	std::array<ScoreSlot, kCategoryCount> preview{};
	int currentTurn = 0;
	int rollCount = 0;
	bool quitRequested = false;

	DiceAnimator animator;
	float time = 0.0f;
	std::array<float, kCategoryCount> rowFlash{};	// 점수 기록 시 행 깜빡임

	// 가상 좌표(1280x720) -> 화면 픽셀 변환
	float uiScale = 1.0f;
	float uiOffsetX = 0.0f;
	float uiOffsetY = 0.0f;
	DirectX::XMFLOAT2 mouse{ -1.0f, -1.0f };
	Hit hover;

	DirectX::XMFLOAT4X4 view{};
	DirectX::XMFLOAT4X4 proj{};
	DirectX::XMFLOAT3 eye{};
	std::array<DirectX::XMFLOAT4X4, 5> diceWorld{};
	std::array<DirectX::XMFLOAT3, 5> dicePos{};
	std::array<bool, 5> diceVisible{};
	std::array<DirectX::XMFLOAT3, 5> diceScreen{};	// 가상 좌표 x, y, 반지름 (클릭 판정/라벨용)
};
