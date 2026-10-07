#include "VisualGame.h"
#include <algorithm>
#include <cmath>
#include <string>

using namespace DirectX;

namespace {
	constexpr float kVirtualW = 1280.0f;
	constexpr float kVirtualH = 720.0f;
	constexpr float kBaseFovY = 0.55f;

	const D2D1_RECT_F kScorePanel = { 24, 24, 424, 696 };
	const D2D1_RECT_F kRollButton = { 740, 614, 940, 674 };
	const D2D1_RECT_F kBackButton = { 540, 590, 740, 646 };
	const D2D1_RECT_F kPlayAgainButton = { 600, 500, 820, 556 };
	const D2D1_RECT_F kToMenuButton = { 860, 500, 1080, 556 };
	const D2D1_RECT_F kEverything = { -4000, -4000, 8000, 8000 };	// 레터박스 영역까지 덮기

	const XMFLOAT4 kDieColor = { 0.97f, 0.95f, 0.89f, 1.0f };

	D2D1_RECT_F MenuButtonRect(int i) {
		const float top = 470.0f + i * 72.0f;
		return { 540, top, 740, top + 58 };
	}

	D2D1_RECT_F CategoryRowRect(int i) {
		const float top = i < 6 ? 76.0f + i * 34.0f : 354.0f + (i - 6) * 34.0f;
		return { 36, top, 412, top + 32 };
	}

	bool Contains(const D2D1_RECT_F& r, XMFLOAT2 p) {
		return p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom;
	}

	const wchar_t* CategoryName(int i) {
		static const wchar_t* names[] = {
			L"Ones", L"Twos", L"Threes", L"Fours", L"Fives", L"Sixes",
			L"Choice", L"Four of a Kind", L"Full House", L"Small Straight", L"Large Straight", L"Yacht",
		};
		return names[i];
	}

	const wchar_t* CategoryDesc(int i) {
		static const wchar_t* descs[] = {
			L"1의 합", L"2의 합", L"3의 합", L"4의 합", L"5의 합", L"6의 합",
			L"모든 눈의 합", L"같은 눈 4개의 합", L"3개 + 2개", L"4연속 · 15점", L"5연속 · 30점", L"5개 동일 · 50점",
		};
		return descs[i];
	}
}

void VisualGame::Initialize(Renderer* r) {
	renderer = r;
	ui.Initialize(renderer->GetD2DContext(), renderer->GetDWriteFactory());
	dice.Reset();
	UpdateLayout();
}

// ---------------------------------------------------------------- 게임 흐름

void VisualGame::StartNewGame() {
	scoreboard = ScoreBoard();
	currentTurn = 0;
	rowFlash.fill(0.0f);
	screen = Screen::Playing;
	StartTurn();
}

void VisualGame::StartTurn() {
	dice.Reset();
	rollCount = 0;
	preview = {};
	animator.Hide();
	turnState = TurnState::WaitRoll;
}

bool VisualGame::CanRoll() const {
	return screen == Screen::Playing && turnState != TurnState::Rolling && rollCount < kMaxRolls;
}

bool VisualGame::CanToggleKeep() const {
	return screen == Screen::Playing && turnState == TurnState::Choosing && rollCount < kMaxRolls;
}

void VisualGame::RollDice() {
	if (!CanRoll()) return;
	dice.Roll_Selected();
	rollCount++;
	preview = Scorer::MakePreviewScores(dice.get_dice_values());
	animator.StartRoll(dice.get_dice_values(), dice.get_keep_status());
	turnState = TurnState::Rolling;
}

void VisualGame::ToggleKeep(int index) {
	if (!CanToggleKeep()) return;
	std::array<bool, 5> keep = dice.get_keep_status();
	keep[index] = !keep[index];
	dice.Select_Keep(keep);
}

void VisualGame::ChooseCategory(int index) {
	if (screen != Screen::Playing || turnState != TurnState::Choosing) return;
	const auto category = static_cast<ScoreCategory>(index);
	if (scoreboard.IsUsed(category)) return;

	scoreboard.SetScore(category, preview[index].score);
	rowFlash[index] = 1.0f;
	currentTurn++;
	if (currentTurn >= kMaxTurns) {
		screen = Screen::GameOver;
		animator.Hide();
	}
	else {
		StartTurn();
	}
}

// ---------------------------------------------------------------- 입력

XMFLOAT2 VisualGame::ToVirtual(int x, int y) const {
	return { (x - uiOffsetX) / uiScale, (y - uiOffsetY) / uiScale };
}

void VisualGame::OnMouseMove(int x, int y) {
	mouse = ToVirtual(x, y);
	hover = HitTest(mouse);
}

void VisualGame::OnMouseDown(int x, int y) {
	mouse = ToVirtual(x, y);
	const Hit hit = HitTest(mouse);
	switch (hit.type) {
	case HitType::Start:     StartNewGame(); break;
	case HitType::HowTo:     screen = Screen::HowTo; break;
	case HitType::Quit:      quitRequested = true; break;
	case HitType::Back:      screen = Screen::MainMenu; break;
	case HitType::Roll:      RollDice(); break;
	case HitType::Die:       ToggleKeep(hit.index); break;
	case HitType::Category:  ChooseCategory(hit.index); break;
	case HitType::PlayAgain: StartNewGame(); break;
	case HitType::ToMenu:    screen = Screen::MainMenu; break;
	default: break;
	}
	hover = HitTest(mouse);
}

void VisualGame::OnKeyDown(WPARAM key) {
	switch (screen) {
	case Screen::MainMenu:
		if (key == VK_RETURN || key == VK_SPACE) StartNewGame();
		else if (key == VK_ESCAPE) quitRequested = true;
		break;
	case Screen::HowTo:
		if (key == VK_ESCAPE || key == VK_RETURN || key == VK_SPACE) screen = Screen::MainMenu;
		break;
	case Screen::Playing:
		if (key == VK_SPACE || key == VK_RETURN) RollDice();
		else if (key >= '1' && key <= '5') ToggleKeep(static_cast<int>(key - '1'));
		else if (key >= VK_NUMPAD1 && key <= VK_NUMPAD5) ToggleKeep(static_cast<int>(key - VK_NUMPAD1));
		break;
	case Screen::GameOver:
		if (key == VK_RETURN || key == VK_SPACE) StartNewGame();
		else if (key == VK_ESCAPE) screen = Screen::MainMenu;
		break;
	}
	hover = HitTest(mouse);
}

VisualGame::Hit VisualGame::HitTest(XMFLOAT2 p) const {
	switch (screen) {
	case Screen::MainMenu:
		for (int i = 0; i < 3; ++i) {
			if (Contains(MenuButtonRect(i), p)) {
				const HitType types[] = { HitType::Start, HitType::HowTo, HitType::Quit };
				return { types[i], i };
			}
		}
		break;
	case Screen::HowTo:
		if (Contains(kBackButton, p)) return { HitType::Back };
		break;
	case Screen::Playing:
		if (CanRoll() && Contains(kRollButton, p)) return { HitType::Roll };
		if (turnState == TurnState::Choosing) {
			for (int i = 0; i < kCategoryCount; ++i) {
				if (!scoreboard.IsUsed(static_cast<ScoreCategory>(i)) && Contains(CategoryRowRect(i), p))
					return { HitType::Category, i };
			}
		}
		if (CanToggleKeep()) {
			for (int i = 0; i < 5; ++i) {
				const XMFLOAT3& s = diceScreen[i];
				const float dx = p.x - s.x, dy = p.y - s.y;
				if (diceVisible[i] && dx * dx + dy * dy <= s.z * s.z) return { HitType::Die, i };
			}
		}
		break;
	case Screen::GameOver:
		if (Contains(kPlayAgainButton, p)) return { HitType::PlayAgain };
		if (Contains(kToMenuButton, p)) return { HitType::ToMenu };
		break;
	}
	return {};
}

bool VisualGame::IsHovered(HitType type, int index) const {
	return hover.type == type && (index < 0 || hover.index == index);
}

// ---------------------------------------------------------------- 갱신

void VisualGame::UpdateLayout() {
	const float w = static_cast<float>(renderer->GetWidth());
	const float h = static_cast<float>(renderer->GetHeight());
	uiScale = (std::min)(w / kVirtualW, h / kVirtualH);
	uiOffsetX = (w - kVirtualW * uiScale) * 0.5f;
	uiOffsetY = (h - kVirtualH * uiScale) * 0.5f;
}

void VisualGame::UpdateCamera() {
	const float w = static_cast<float>(renderer->GetWidth());
	const float h = static_cast<float>(renderer->GetHeight());
	const float aspect = w / h;

	eye = { 0.0f, 21.0f, -11.0f };
	const XMMATRIX viewM = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMVectorSet(0, 0, -1, 1), XMVectorSet(0, 1, 0, 0));

	// 창이 16:9보다 좁으면 UI처럼 가로 기준으로 맞추도록 시야각을 넓힌다
	float fovY = kBaseFovY;
	const float virtualAspect = kVirtualW / kVirtualH;
	if (aspect < virtualAspect) fovY = 2.0f * std::atan(std::tan(kBaseFovY * 0.5f) * virtualAspect / aspect);

	// 카메라 주시점이 UI의 특정 가상 좌표에 오도록 투영을 평행이동(off-center)
	const XMFLOAT2 anchor = (screen == Screen::MainMenu || screen == Screen::HowTo)
		? XMFLOAT2{ 640.0f, 395.0f } : XMFLOAT2{ 840.0f, 320.0f };
	const float px = uiOffsetX + anchor.x * uiScale;
	const float py = uiOffsetY + anchor.y * uiScale;
	const float ndcX = px / w * 2.0f - 1.0f;
	const float ndcY = 1.0f - py / h * 2.0f;
	const XMMATRIX projM = XMMatrixPerspectiveFovLH(fovY, aspect, 0.5f, 200.0f) * XMMatrixTranslation(ndcX, ndcY, 0.0f);

	XMStoreFloat4x4(&view, viewM);
	XMStoreFloat4x4(&proj, projM);
}

void VisualGame::UpdateDiceTransforms() {
	const bool showcase = (screen == Screen::MainMenu || screen == Screen::HowTo);
	const XMMATRIX viewM = XMLoadFloat4x4(&view);
	const XMMATRIX projM = XMLoadFloat4x4(&proj);
	const float w = static_cast<float>(renderer->GetWidth());
	const float h = static_cast<float>(renderer->GetHeight());

	for (int i = 0; i < 5; ++i) {
		XMMATRIX world;
		if (showcase) {
			// 메인 메뉴: 공중에서 천천히 회전하는 주사위
			const float bob = std::sin(time * 1.6f + i * 0.9f) * 0.35f;
			dicePos[i] = { (i - 2) * 2.6f, 1.8f + bob, 0.0f };
			world = XMMatrixRotationRollPitchYaw(time * 0.7f + i * 1.1f, time * 0.9f + i * 0.7f, time * 0.3f + i)
				* XMMatrixTranslation(dicePos[i].x, dicePos[i].y, dicePos[i].z);
			diceVisible[i] = true;
		}
		else {
			dicePos[i] = animator.GetPosition(i);
			world = animator.GetWorld(i);
			diceVisible[i] = animator.IsVisible(i);
		}
		XMStoreFloat4x4(&diceWorld[i], world);

		// 화면에 투영해서 클릭 판정 원(가상 좌표)을 구한다
		const XMVECTOR center = XMLoadFloat3(&dicePos[i]);
		const XMVECTOR c = XMVector3Project(center, 0, 0, w, h, 0, 1, projM, viewM, XMMatrixIdentity());
		const XMVECTOR e = XMVector3Project(center + XMVectorSet(1.2f, 0, 0, 0), 0, 0, w, h, 0, 1, projM, viewM, XMMatrixIdentity());
		diceScreen[i] = {
			(XMVectorGetX(c) - uiOffsetX) / uiScale,
			(XMVectorGetY(c) - uiOffsetY) / uiScale,
			XMVectorGetX(XMVector2Length(e - c)) / uiScale,
		};
	}
}

void VisualGame::Update(float dt) {
	time += dt;
	UpdateLayout();
	UpdateCamera();

	animator.Update(dt, dice.get_keep_status());
	if (turnState == TurnState::Rolling && !animator.IsRolling()) {
		turnState = TurnState::Choosing;
	}
	for (float& f : rowFlash) f = (std::max)(0.0f, f - dt * 1.5f);

	UpdateDiceTransforms();
	hover = HitTest(mouse);
}

// ---------------------------------------------------------------- 그리기

void VisualGame::Render() {
	FrameConstants frame{};
	XMStoreFloat4x4(&frame.viewProj, XMMatrixTranspose(XMLoadFloat4x4(&view) * XMLoadFloat4x4(&proj)));
	frame.eyePos = { eye.x, eye.y, eye.z, 1.0f };
	XMStoreFloat4(&frame.lightDir, XMVector3Normalize(XMVectorSet(-0.45f, 1.0f, -0.55f, 0.0f)));
	for (int i = 0; i < 5; ++i) {
		frame.dicePos[i] = { dicePos[i].x, dicePos[i].y, dicePos[i].z, diceVisible[i] ? 1.0f : 0.0f };
	}

	if (!renderer->BeginFrame(frame)) return;

	renderer->DrawTable();
	for (int i = 0; i < 5; ++i) {
		if (!diceVisible[i]) continue;
		const bool showcase = (screen == Screen::MainMenu || screen == Screen::HowTo);
		const float highlight = IsHovered(HitType::Die, i) ? 1.0f : 0.0f;
		const float keep = showcase ? 0.0f : animator.GetKeepAmount(i);
		renderer->DrawDie(XMLoadFloat4x4(&diceWorld[i]), kDieColor, highlight, keep);
	}

	renderer->BeginUi();
	ui.Begin(uiScale, uiOffsetX, uiOffsetY);
	switch (screen) {
	case Screen::MainMenu: DrawMainMenu(); break;
	case Screen::HowTo:    DrawHowTo(); break;
	case Screen::Playing:  DrawPlaying(); break;
	case Screen::GameOver: DrawGameOver(); break;
	}
	renderer->EndFrame();
}

void VisualGame::DrawMainMenu() {
	ui.Text(L"YACHT DICE", { 0, 60, kVirtualW, 170 }, TextStyle::Title, Rgb(0xFFFFFF), DWRITE_TEXT_ALIGNMENT_CENTER);
	ui.Text(L"DirectX 12 Edition", { 0, 165, kVirtualW, 195 }, TextStyle::Small, Rgb(0xE8B33A), DWRITE_TEXT_ALIGNMENT_CENTER);

	const wchar_t* labels[] = { L"게임 시작", L"게임 방법", L"종료" };
	const HitType types[] = { HitType::Start, HitType::HowTo, HitType::Quit };
	for (int i = 0; i < 3; ++i) {
		ui.Button(MenuButtonRect(i), labels[i], IsHovered(types[i]), true, i == 0);
	}
	ui.Text(L"Space: 굴리기   ·   1~5 / 클릭: 주사위 고정   ·   점수판 클릭: 점수 기록",
		{ 0, 684, kVirtualW, 710 }, TextStyle::Small, Rgb(0xB8C4CF, 0.8f), DWRITE_TEXT_ALIGNMENT_CENTER);
}

void VisualGame::DrawHowTo() {
	ui.FillRect(kEverything, Rgb(0x000000, 0.55f));
	const D2D1_RECT_F panel = { 160, 56, 1120, 664 };
	ui.FillRect(panel, Rgb(0x111A23, 0.96f), 18.0f);
	ui.StrokeRect(panel, Rgb(0xE8B33A, 0.5f), 18.0f, 1.5f);
	ui.Text(L"게임 방법", { 200, 76, 1080, 120 }, TextStyle::Heading, Rgb(0xE8B33A));

	const wchar_t* rules =
		L"•  한 게임은 총 12턴으로 진행됩니다.\n"
		L"•  한 턴에서 주사위는 최대 3번까지 굴릴 수 있습니다.\n"
		L"•  주사위를 클릭(또는 숫자키 1~5)하면 고정(KEEP)되어, 다시 굴릴 때 그대로 유지됩니다.\n"
		L"•  매 턴이 끝나면 아직 사용하지 않은 카테고리 하나를 점수판에서 골라 점수를 기록합니다.\n"
		L"\n"
		L"상단  —  Ones ~ Sixes : 해당 눈이 나온 주사위의 합\n"
		L"하단  —  Choice : 모든 눈의 합\n"
		L"            Four of a Kind : 같은 눈 4개의 합\n"
		L"            Full House : 같은 눈 3개 + 2개 (모든 눈의 합)\n"
		L"            Small Straight : 4개 연속 (15점)   ·   Large Straight : 5개 연속 (30점)\n"
		L"            Yacht : 5개 모두 같은 눈 (50점)\n"
		L"\n"
		L"•  상단 합계가 63점 이상이면 보너스 35점을 획득합니다.\n"
		L"•  12턴이 모두 끝나면 게임이 종료되고 최종 점수가 집계됩니다.";
	ui.Text(rules, { 200, 130, 1080, 570 }, TextStyle::Body, Rgb(0xE6ECF2), DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	ui.Button(kBackButton, L"돌아가기", IsHovered(HitType::Back), true, true);
}

void VisualGame::DrawScoreBoard() {
	ui.FillRect(kScorePanel, Rgb(0x0E151C, 0.86f), 16.0f);
	ui.StrokeRect(kScorePanel, Rgb(0xFFFFFF, 0.08f), 16.0f, 1.0f);
	ui.Text(L"SCORE BOARD", { 48, 32, 400, 70 }, TextStyle::Heading, Rgb(0xFFFFFF));

	const bool choosing = (screen == Screen::Playing && turnState == TurnState::Choosing);
	for (int i = 0; i < kCategoryCount; ++i) {
		const auto cat = static_cast<ScoreCategory>(i);
		const D2D1_RECT_F row = CategoryRowRect(i);
		const bool used = scoreboard.IsUsed(cat);
		const bool selectable = choosing && !used;
		const bool hovered = selectable && IsHovered(HitType::Category, i);

		if (hovered) ui.FillRect(row, Rgb(0xE8B33A, 0.28f), 8.0f);
		else if (i % 2 == 0) ui.FillRect(row, Rgb(0xFFFFFF, 0.035f), 8.0f);
		if (rowFlash[i] > 0.0f) ui.FillRect(row, Rgb(0xFFD36A, 0.55f * rowFlash[i]), 8.0f);

		const D2D1_COLOR_F nameColor = used ? Rgb(0x8A96A3) : Rgb(0xFFFFFF);
		ui.Text(CategoryName(i), { row.left + 12, row.top, row.left + 160, row.bottom }, TextStyle::Label, nameColor);
		ui.Text(CategoryDesc(i), { row.left + 160, row.top, row.right - 60, row.bottom }, TextStyle::Small, Rgb(0x7D8A97));

		std::wstring scoreText = L"-";
		D2D1_COLOR_F scoreColor = Rgb(0x56616C);
		if (used) {
			scoreText = std::to_wstring(scoreboard.GetScore(cat));
			scoreColor = Rgb(0xFFFFFF);
		}
		else if (selectable) {
			const int value = preview[i].score;
			scoreText = std::to_wstring(value);
			scoreColor = value > 0 ? Rgb(0xFFCF5A) : Rgb(0x8C7A55);
		}
		ui.Text(scoreText, { row.right - 70, row.top, row.right - 12, row.bottom }, TextStyle::Score, scoreColor, DWRITE_TEXT_ALIGNMENT_TRAILING);
	}

	// 상단 합계 + 보너스 진행도
	const int subtotal = scoreboard.GetSubTotalScore();
	const int bonus = scoreboard.GetBonus();
	ui.Text(L"상단 합계", { 48, 290, 250, 316 }, TextStyle::Label, Rgb(0xC9D3DC));
	ui.Text(std::to_wstring(subtotal) + L" / 63", { 250, 290, 400, 316 }, TextStyle::Score, Rgb(0xC9D3DC), DWRITE_TEXT_ALIGNMENT_TRAILING);
	const D2D1_RECT_F bar = { 48, 326, 290, 334 };
	ui.FillRect(bar, Rgb(0xFFFFFF, 0.1f), 4.0f);
	const float ratio = (std::min)(1.0f, subtotal / 63.0f);
	if (ratio > 0.0f) ui.FillRect({ bar.left, bar.top, bar.left + (bar.right - bar.left) * ratio, bar.bottom }, Rgb(0xE8B33A), 4.0f);
	ui.Text(bonus > 0 ? L"보너스 +35" : L"보너스 +0", { 290, 318, 400, 342 }, TextStyle::Small,
		bonus > 0 ? Rgb(0xFFCF5A) : Rgb(0x7D8A97), DWRITE_TEXT_ALIGNMENT_TRAILING);

	// 총점 (GetTotalScore가 보너스를 포함)
	ui.Line({ 48, 572 }, { 400, 572 }, Rgb(0xFFFFFF, 0.12f), 1.0f);
	ui.Text(L"TOTAL", { 48, 582, 220, 680 }, TextStyle::Heading, Rgb(0xFFFFFF));
	ui.Text(std::to_wstring(scoreboard.GetTotalScore()),{ 200, 582, 400, 680 }, TextStyle::Big,
		Rgb(0xFFCF5A), DWRITE_TEXT_ALIGNMENT_TRAILING);
}

void VisualGame::DrawPlaying() {
	DrawScoreBoard();

	// 상단: 턴 / 굴린 횟수
	ui.Text(L"TURN  " + std::to_wstring(currentTurn + 1) + L" / " + std::to_wstring(kMaxTurns),
		{ 460, 28, 900, 76 }, TextStyle::Heading, Rgb(0xFFFFFF));
	ui.Text(L"ROLLS", { 1000, 28, 1130, 76 }, TextStyle::Small, Rgb(0xB8C4CF), DWRITE_TEXT_ALIGNMENT_TRAILING);
	for (int i = 0; i < kMaxRolls; ++i) {
		const D2D1_POINT_2F c = { 1158.0f + i * 34.0f, 52.0f };
		if (i < rollCount) ui.FillCircle(c, 11.0f, Rgb(0xE8B33A));
		else ui.StrokeCircle(c, 11.0f, Rgb(0xB8C4CF, 0.6f), 2.0f);
	}

	// 주사위 아래 라벨: KEEP 표시 또는 단축키 번호
	if (turnState == TurnState::Choosing) {
		for (int i = 0; i < 5; ++i) {
			if (!diceVisible[i]) continue;
			const XMFLOAT3& s = diceScreen[i];
			const float y = s.y + s.z * 0.9f + 10.0f;
			if (dice.get_keep_status()[i]) {
				const D2D1_RECT_F pill = { s.x - 34, y, s.x + 34, y + 24 };
				ui.FillRect(pill, Rgb(0xE8B33A), 12.0f);
				ui.Text(L"KEEP", pill, TextStyle::Small, Rgb(0x1B1406), DWRITE_TEXT_ALIGNMENT_CENTER);
			}
			else if (CanToggleKeep()) {
				ui.Text(std::to_wstring(i + 1), { s.x - 20, y, s.x + 20, y + 24 }, TextStyle::Small,
					Rgb(0xFFFFFF, 0.55f), DWRITE_TEXT_ALIGNMENT_CENTER);
			}
		}
	}

	// 안내 문구
	std::wstring hint;
	if (turnState == TurnState::WaitRoll) hint = L"ROLL 버튼(Space)을 눌러 주사위를 굴리세요";
	else if (turnState == TurnState::Choosing && rollCount < kMaxRolls) hint = L"주사위를 클릭해 고정(KEEP)하고 다시 굴리거나, 점수판에서 카테고리를 선택하세요";
	else if (turnState == TurnState::Choosing) hint = L"굴릴 기회를 모두 사용했습니다. 점수판에서 카테고리를 선택하세요";
	if (!hint.empty()) {
		ui.Text(hint, { 440, 568, 1240, 598 }, TextStyle::Small, Rgb(0xE6ECF2, 0.85f), DWRITE_TEXT_ALIGNMENT_CENTER);
	}

	const std::wstring rollLabel = rollCount < kMaxRolls
		? L"ROLL  (" + std::to_wstring(kMaxRolls - rollCount) + L")" : L"ROLL";
	ui.Button(kRollButton, rollLabel, IsHovered(HitType::Roll), CanRoll(), true);
}

void VisualGame::DrawGameOver() {
	DrawScoreBoard();

	const D2D1_RECT_F panel = { 520, 130, 1160, 590 };
	ui.FillRect(panel, Rgb(0x111A23, 0.94f), 18.0f);
	ui.StrokeRect(panel, Rgb(0xE8B33A, 0.6f), 18.0f, 1.5f);

	const int upper = scoreboard.GetSubTotalScore();
	const int bonus = scoreboard.GetBonus();
	const int total = scoreboard.GetTotalScore();	// 상단 + 보너스 + 하단
	const int lower = total - upper - bonus;
	ui.Text(L"GAME OVER", { 520, 160, 1160, 210 }, TextStyle::Heading, Rgb(0xFFFFFF), DWRITE_TEXT_ALIGNMENT_CENTER);
	ui.Text(L"최종 점수", { 520, 230, 1160, 260 }, TextStyle::Body, Rgb(0xB8C4CF), DWRITE_TEXT_ALIGNMENT_CENTER);
	ui.Text(std::to_wstring(total),{ 520, 262, 1160, 360 }, TextStyle::Title, Rgb(0xFFCF5A), DWRITE_TEXT_ALIGNMENT_CENTER);
	ui.Text(L"상단 " + std::to_wstring(upper) + L"   +   보너스 " + std::to_wstring(bonus) + L"   +   하단 " + std::to_wstring(lower),
		{ 520, 390, 1160, 430 }, TextStyle::Body, Rgb(0xE6ECF2), DWRITE_TEXT_ALIGNMENT_CENTER);

	ui.Button(kPlayAgainButton, L"다시 하기", IsHovered(HitType::PlayAgain), true, true);
	ui.Button(kToMenuButton, L"메인 메뉴", IsHovered(HitType::ToMenu), true, false);
}
