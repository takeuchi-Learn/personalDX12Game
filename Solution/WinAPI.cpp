#include "WinAPI.h"

#include <imgui_impl_win32.h>
#include <cmath>

// 通常スタイルからサイズ変更を無くしたスタイル
const DWORD	WinAPI::windowStyle = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WinAPI::WinAPI()
	: windowSize({ window_width, window_height })
{
	// 初期状態のウィンドウタイトル
	constexpr wchar_t winTitleDef[] = L"DX12Game (SE : OtoLogic)";

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc; // ウィンドウプロシージャを設定
	w.lpszClassName = winTitleDef; // ウィンドウクラス名
	w.hInstance = GetModuleHandle(nullptr); // ウィンドウハンドル
	w.hCursor = LoadCursor(NULL, IDC_ARROW); // カーソル指定

	// ウィンドウクラスをOSに登録
	RegisterClassEx(&w);
	// ウィンドウサイズ{ X座標 Y座標 横幅 縦幅 }
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, windowStyle, false); // 自動でサイズ補正

	// ウィンドウオブジェクトの生成
	hwnd = CreateWindow(w.lpszClassName,		// クラス名
						winTitleDef,			// タイトルバーの文字
						windowStyle,			// ウィンドウスタイル
						CW_USEDEFAULT,			// 表示X座標（OSに任せる）
						CW_USEDEFAULT,			// 表示Y座標（OSに任せる）
						wrc.right - wrc.left,	// ウィンドウ横幅
						wrc.bottom - wrc.top,	// ウィンドウ縦幅
						nullptr,				// 親ウィンドウハンドル
						nullptr,				// メニューハンドル
						w.hInstance,			// 呼び出しアプリケーションハンドル
						nullptr);				// オプション

	// ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);
}

bool WinAPI::setWindowSize(int sizeX, int sizeY, const POINT* pos, bool bRepaint)
{
	POINT winPos{};

	if (pos == nullptr)
	{
		WINDOWINFO wInfo{};
		GetWindowInfo(hwnd, &wInfo);

		winPos.x = wInfo.rcWindow.left;
		winPos.y = wInfo.rcWindow.top;
	} else
	{
		winPos = *pos;
	}

	RECT wrc = { 0, 0, sizeX, sizeY };
	AdjustWindowRect(&wrc, windowStyle, false); // 自動でサイズ補正

	const bool ret
		= MoveWindow(hwnd,
					 winPos.x,
					 winPos.y,
					 wrc.right - wrc.left,
					 wrc.bottom - wrc.top,
					 bRepaint ? TRUE : FALSE)
		? true : false;

	windowSize.x = sizeX;
	windowSize.y = sizeY;

	return ret;
}

bool WinAPI::setWindowWidth(int sizeX)
{
	const float raito = (float)windowSize.y / (float)windowSize.x;

	return setWindowSize(sizeX, (int)std::round(sizeX * raito));
}

bool WinAPI::setWindowHeight(int sizeY)
{
	const float raito = (float)windowSize.x / (float)windowSize.y;

	return setWindowSize((int)std::roundf(sizeY * raito), sizeY);
}

WinAPI::~WinAPI()
{
	// ウィンドウクラスを登録解除
	UnregisterClass(w.lpszClassName, w.hInstance);
}

LRESULT WinAPI::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return 1;
	}

	// メッセージで分岐
	switch (msg)
	{
	case WM_DESTROY: // ウィンドウが破棄された
		PostQuitMessage(0); // OSに対して、アプリの終了を伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); // 標準の処理を行う
}

WinAPI* WinAPI::getInstance()
{
	static WinAPI winApi{};
	return &winApi;
}

HWND WinAPI::getHwnd() { return hwnd; }
WNDCLASSEX WinAPI::getW() { return w; }

void WinAPI::setWindowText(const LPCSTR window_title)
{
	SetWindowTextA(hwnd, window_title);
}

bool WinAPI::processMessage()
{
	MSG msg{};  // メッセージ

	// メッセージがある？
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg); // キー入力メッセージの処理
		DispatchMessage(&msg); // プロシージャにメッセージを送る
	}

	// ×ボタンで終了メッセージが来たらゲームループを抜ける
	if (msg.message == WM_QUIT)
	{
		return true;
	}
	return false;
}