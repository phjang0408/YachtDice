#pragma once
#include <d2d1_3.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>

enum class TextStyle { Title, Heading, Body, Small, Label, Score, Big, Button, COUNT };

// Direct2D/DirectWrite 그리기 도우미. 모든 좌표는 1280x720 가상 좌표계 기준이다.
class Ui {
public:
	void Initialize(ID2D1DeviceContext2* context, IDWriteFactory* dwrite);
	void Begin(float scale, float offsetX, float offsetY);

	void FillRect(const D2D1_RECT_F& r, const D2D1_COLOR_F& c, float radius = 0.0f);
	void StrokeRect(const D2D1_RECT_F& r, const D2D1_COLOR_F& c, float radius, float width);
	void FillCircle(D2D1_POINT_2F center, float radius, const D2D1_COLOR_F& c);
	void StrokeCircle(D2D1_POINT_2F center, float radius, const D2D1_COLOR_F& c, float width);
	void Line(D2D1_POINT_2F a, D2D1_POINT_2F b, const D2D1_COLOR_F& c, float width);
	void Text(const std::wstring& text, const D2D1_RECT_F& r, TextStyle style, const D2D1_COLOR_F& c,
		DWRITE_TEXT_ALIGNMENT align = DWRITE_TEXT_ALIGNMENT_LEADING,
		DWRITE_PARAGRAPH_ALIGNMENT valign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	void Button(const D2D1_RECT_F& r, const std::wstring& label, bool hovered, bool enabled, bool primary);

private:
	ID2D1DeviceContext2* ctx = nullptr;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> formats[static_cast<int>(TextStyle::COUNT)];
};

inline D2D1_COLOR_F Rgb(UINT32 rgb, float alpha = 1.0f) { return D2D1::ColorF(rgb, alpha); }
