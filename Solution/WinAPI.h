#pragma once
#include <Windows.h>

class WinAPI {
private:
	WinAPI(const WinAPI &winapi) = delete;
	WinAPI &operator=(const WinAPI &winapi) = delete;

	WinAPI();
	~WinAPI();

	WNDCLASSEX w{}; // ウィンドウクラスの設定
	HWND hwnd;

	static const DWORD windowStyle;

	//ウィンドウプロシージャ
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


	bool setWindowSize(int sizeX, int sizeY, bool bRepaint = true);

public:
	const static int window_width = 1280;
	const static int window_height = 720;

	// アスペクト比固定でウィンドウサイズを変更
	bool setWindowWidth(int sizeX);
	// アスペクト比固定でウィンドウサイズを変更
	bool setWindowHeight(int sizeY);

	POINT getWindowSIze();

	static WinAPI *getInstance();

	HWND getHwnd();
	WNDCLASSEX getW();

	void setWindowText(const LPCSTR window_title);

	//ゲームループ内で毎回呼びだす。trueが返ってきたらすぐに終了させるべき
	//@return 異常の有無をbool型で返す(true == 異常 , false == 正常)
	bool processMessage();
};

