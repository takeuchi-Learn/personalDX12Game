#pragma once

#include <Windows.h>

#include <wrl.h>	//ComPtr

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

class Input {
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	struct MouseMove {
		LONG x;
		LONG y;
		LONG wheel;
	};

private:
	Input(const Input& ip) = delete;
	Input& operator=(const Input& ip) = delete;

	Input();
	~Input();

	BYTE key[256];
	BYTE preKey[256];

	DIMOUSESTATE2 mouseState{};
	DIMOUSESTATE2 preMouseState{};

	POINT mousePos{};

	ComPtr<IDirectInputDevice8> devkeyboard;
	ComPtr<IDirectInputDevice8> devmouse;

	ComPtr<IDirectInput8> dinput;

public:
	static Input* getInstance();

	void init();
	void update();

	bool hitKey(BYTE keyCode);
	bool hitPreKey(BYTE keyCode);
	bool triggerKey(BYTE keyCode);

	void resetState();

	enum MOUSE : BYTE {
		LEFT = 0,
		RIGHT = 1,
		WHEEL = 2
	};

	// @param keyCode Input::MOUSE::なんとか、rgbButtons配列の添え字
	bool hitMouseBotton(_In_ BYTE keyCode);
	// @param keyCode Input::MOUSE::なんとか、rgbButtons配列の添え字
	bool hitPreMouseBotton(_In_ BYTE keyCode);
	// @param keyCode Input::MOUSE::なんとか、rgbButtons配列の添え字
	bool triggerMouseBotton(_In_ BYTE keyCode);

	MouseMove getMouseMove();

	inline LONG getMouseWheelScroll() { return mouseState.lZ; }

	// @return POINT型(LONG x とLONG y のみの構造体)
	POINT getMousePos();
	bool setMousePos(int x, int y);

	void changeDispMouseCursorFlag(const  bool dispFlag);
};

