#include "WinAPI.h"

#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WinAPI::WinAPI() {
	constexpr wchar_t winTitleDef[] = L"DX12Game";

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc; // ウィンドウプロシージャを設定
	w.lpszClassName = winTitleDef; // ウィンドウクラス名
	w.hInstance = GetModuleHandle(nullptr); // ウィンドウハンドル
	w.hCursor = LoadCursor(NULL, IDC_ARROW); // カーソル指定

	// ウィンドウクラスをOSに登録
	RegisterClassEx(&w);
	// ウィンドウサイズ{ X座標 Y座標 横幅 縦幅 }
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); // 自動でサイズ補正

	// ウィンドウオブジェクトの生成
	hwnd = CreateWindow(w.lpszClassName, // クラス名
		winTitleDef,         // タイトルバーの文字
		WS_OVERLAPPEDWINDOW,        // 標準的なウィンドウスタイル
		CW_USEDEFAULT,              // 表示X座標（OSに任せる）
		CW_USEDEFAULT,              // 表示Y座標（OSに任せる）
		wrc.right - wrc.left,       // ウィンドウ横幅
		wrc.bottom - wrc.top,   // ウィンドウ縦幅
		nullptr,                // 親ウィンドウハンドル
		nullptr,                // メニューハンドル
		w.hInstance,            // 呼び出しアプリケーションハンドル
		nullptr);               // オプション

	// ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);
}

WinAPI::~WinAPI() {
	// ウィンドウクラスを登録解除
	UnregisterClass(w.lpszClassName, w.hInstance);
}

LRESULT WinAPI::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return 1;
	}

	// メッセージで分岐
	switch (msg) {
	case WM_DESTROY: // ウィンドウが破棄された
		PostQuitMessage(0); // OSに対して、アプリの終了を伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); // 標準の処理を行う
}

WinAPI* WinAPI::getInstance() {
	static WinAPI winApi{};
	return &winApi;
}

HWND WinAPI::getHwnd() { return hwnd; }
WNDCLASSEX WinAPI::getW() { return w; }

void WinAPI::setWindowText(const LPCSTR window_title) {
	SetWindowTextA(hwnd, window_title);
}

bool WinAPI::processMessage() {
	MSG msg{};  // メッセージ

	// メッセージがある？
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg); // キー入力メッセージの処理
		DispatchMessage(&msg); // プロシージャにメッセージを送る
	}

	// ×ボタンで終了メッセージが来たらゲームループを抜ける
	if (msg.message == WM_QUIT) {
		return true;
	}
	return false;
}