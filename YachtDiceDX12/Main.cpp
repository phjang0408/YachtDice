#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <exception>
#include "Renderer.h"
#include "VisualGame.h"

namespace {
	Renderer g_renderer;
	VisualGame g_game;
	bool g_ready = false;

	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
		case WM_SIZE:
			if (g_ready) g_renderer.Resize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		case WM_MOUSEMOVE:
			if (g_ready) g_game.OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_LBUTTONDOWN:
			if (g_ready) g_game.OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_KEYDOWN:
			if (g_ready && !(lParam & (1 << 30))) g_game.OnKeyDown(wParam);	// 키 반복 입력은 무시
			return 0;
		case WM_SETCURSOR:
			if (LOWORD(lParam) == HTCLIENT) {
				const bool hand = g_ready && g_game.IsHoveringClickable();
				SetCursor(LoadCursor(nullptr, hand ? IDC_HAND : IDC_ARROW));
				return TRUE;
			}
			break;
		case WM_DPICHANGED: {
			const RECT* r = reinterpret_cast<const RECT*>(lParam);
			SetWindowPos(hwnd, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
			return 0;
		}
		case WM_GETMINMAXINFO:
			reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize = { 640, 400 };
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCmd) {
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = L"YachtDiceDX12";
	RegisterClassExW(&wc);

	// 1280x720(96 DPI 기준)을 현재 DPI에 맞추되, 작업 영역을 넘지 않게
	const UINT dpi = GetDpiForSystem();
	RECT work{};
	SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
	int clientW = MulDiv(1280, dpi, 96);
	int clientH = MulDiv(720, dpi, 96);
	const float fit = (std::min)(1.0f, (std::min)((work.right - work.left) * 0.9f / clientW, (work.bottom - work.top) * 0.9f / clientH));
	clientW = static_cast<int>(clientW * fit);
	clientH = static_cast<int>(clientH * fit);
	RECT rc{ 0, 0, clientW, clientH };
	AdjustWindowRectExForDpi(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0, dpi);

	HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Yacht Dice - DirectX 12", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, instance, nullptr);
	if (!hwnd) return 1;

	try {
		RECT client{};
		GetClientRect(hwnd, &client);
		g_renderer.Initialize(hwnd, client.right - client.left, client.bottom - client.top);
		g_game.Initialize(&g_renderer);
		g_ready = true;
		ShowWindow(hwnd, showCmd);

		LARGE_INTEGER freq, prev, now;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&prev);

		MSG msg{};
		while (msg.message != WM_QUIT) {
			if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				continue;
			}
			if (IsIconic(hwnd)) {	// 최소화 중에는 CPU를 쓰지 않도록 대기
				WaitMessage();
				QueryPerformanceCounter(&prev);
				continue;
			}
			QueryPerformanceCounter(&now);
			const float dt = (std::min)(0.1f, static_cast<float>(now.QuadPart - prev.QuadPart) / freq.QuadPart);
			prev = now;

			g_game.Update(dt);
			g_game.Render();
			if (g_game.WantsQuit()) {
				DestroyWindow(hwnd);
				break;
			}
		}
		g_ready = false;
		g_renderer.Shutdown();
	}
	catch (const std::exception& e) {
		g_ready = false;
		MessageBoxA(hwnd, e.what(), "Yacht Dice", MB_ICONERROR | MB_OK);
		return 1;
	}
	return 0;
}
