#include "Ui.h"

void Ui::Initialize(ID2D1DeviceContext2* context, IDWriteFactory* dwrite) {
	ctx = context;
	ctx->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush);

	struct Def { DWRITE_FONT_WEIGHT weight; float size; };
	const Def defs[] = {
		{ DWRITE_FONT_WEIGHT_BLACK,     84.0f },	// Title
		{ DWRITE_FONT_WEIGHT_BOLD,      28.0f },	// Heading
		{ DWRITE_FONT_WEIGHT_NORMAL,    19.0f },	// Body
		{ DWRITE_FONT_WEIGHT_NORMAL,    14.0f },	// Small
		{ DWRITE_FONT_WEIGHT_SEMI_BOLD, 19.0f },	// Label
		{ DWRITE_FONT_WEIGHT_BOLD,      20.0f },	// Score
		{ DWRITE_FONT_WEIGHT_BLACK,     56.0f },	// Big
		{ DWRITE_FONT_WEIGHT_BOLD,      22.0f },	// Button
	};
	for (int i = 0; i < static_cast<int>(TextStyle::COUNT); ++i) {
		// 한글은 DirectWrite 글꼴 대체(맑은 고딕)로 표시된다
		dwrite->CreateTextFormat(L"Segoe UI", nullptr, defs[i].weight, DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL, defs[i].size, L"ko-kr", &formats[i]);
		formats[i]->SetWordWrapping(i == static_cast<int>(TextStyle::Body)
			? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
	}
}

void Ui::Begin(float scale, float offsetX, float offsetY) {
	ctx->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale) * D2D1::Matrix3x2F::Translation(offsetX, offsetY));
}

void Ui::FillRect(const D2D1_RECT_F& r, const D2D1_COLOR_F& c, float radius) {
	brush->SetColor(c);
	if (radius > 0.0f) ctx->FillRoundedRectangle(D2D1::RoundedRect(r, radius, radius), brush.Get());
	else ctx->FillRectangle(r, brush.Get());
}

void Ui::StrokeRect(const D2D1_RECT_F& r, const D2D1_COLOR_F& c, float radius, float width) {
	brush->SetColor(c);
	ctx->DrawRoundedRectangle(D2D1::RoundedRect(r, radius, radius), brush.Get(), width);
}

void Ui::FillCircle(D2D1_POINT_2F center, float radius, const D2D1_COLOR_F& c) {
	brush->SetColor(c);
	ctx->FillEllipse(D2D1::Ellipse(center, radius, radius), brush.Get());
}

void Ui::StrokeCircle(D2D1_POINT_2F center, float radius, const D2D1_COLOR_F& c, float width) {
	brush->SetColor(c);
	ctx->DrawEllipse(D2D1::Ellipse(center, radius, radius), brush.Get(), width);
}

void Ui::Line(D2D1_POINT_2F a, D2D1_POINT_2F b, const D2D1_COLOR_F& c, float width) {
	brush->SetColor(c);
	ctx->DrawLine(a, b, brush.Get(), width);
}

void Ui::Text(const std::wstring& text, const D2D1_RECT_F& r, TextStyle style, const D2D1_COLOR_F& c,
	DWRITE_TEXT_ALIGNMENT align, DWRITE_PARAGRAPH_ALIGNMENT valign) {
	IDWriteTextFormat* format = formats[static_cast<int>(style)].Get();
	format->SetTextAlignment(align);
	format->SetParagraphAlignment(valign);
	brush->SetColor(c);
	ctx->DrawText(text.c_str(), static_cast<UINT32>(text.size()), format, r, brush.Get());
}

void Ui::Button(const D2D1_RECT_F& r, const std::wstring& label, bool hovered, bool enabled, bool primary) {
	D2D1_COLOR_F fill, textColor;
	if (!enabled) {
		fill = Rgb(0x262B32, 0.85f);
		textColor = Rgb(0x666D76);
	}
	else if (primary) {
		fill = hovered ? Rgb(0xFFCF5A) : Rgb(0xE8B33A);
		textColor = Rgb(0x1B1406);
	}
	else {
		fill = hovered ? Rgb(0x3B5068, 0.95f) : Rgb(0x223140, 0.92f);
		textColor = Rgb(0xFFFFFF);
	}
	FillRect(r, fill, 12.0f);
	if (enabled && !primary) StrokeRect(r, Rgb(0x7F9BB8, hovered ? 0.9f : 0.45f), 12.0f, 1.5f);
	Text(label, r, TextStyle::Button, textColor, DWRITE_TEXT_ALIGNMENT_CENTER);
}
