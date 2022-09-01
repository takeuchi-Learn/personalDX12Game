#pragma once

#include <Windows.h>

#include <wrl.h>	//ComPtr

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#include <DirectXMath.h>

class Input {
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	struct MouseMove {
		LONG x;
		LONG y;
		LONG wheel;
	};

private:
	Input(const Input &ip) = delete;
	Input &operator=(const Input &ip) = delete;

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
	static Input *getInstance();

	void init();
	void update();

	bool hitKey(BYTE keyCode);
	bool hitPreKey(BYTE keyCode);
	bool triggerKey(BYTE keyCode);

	void resetState();

	enum MOUSE : BYTE {
		LEFT = 0,
		RIGHT = 1,
		WHEEL = 2,
		BUTTON3 = 3,
		BUTTON4 = 4,
		BUTTON5 = 5,
		BUTTON6 = 6,
		BUTTON7 = 7,
	};

	// @param keyCode Input::MOUSE::なんとか、rgbButtons配列の添え字
	inline bool hitMouseButton(_In_ BYTE keyCode) {
		return (bool)mouseState.rgbButtons[keyCode];
	}

	// @param keyCode Input::MOUSE::なんとか、rgbButtons配列の添え字
	inline bool hitPreMouseButton(_In_ BYTE keyCode) {
		return (bool)preMouseState.rgbButtons[keyCode];
	}

	/// <summary>
	/// マウスのボタンを押した瞬間を検知
	/// </summary>
	/// <param name="keyCode">Input::MOUSE::なんとか、rgbButtons配列の添え字</param>
	/// <returns>を押した瞬間ならtrue</returns>
	inline bool triggerMouseButton(_In_ BYTE keyCode) {
		return hitMouseButton(keyCode) && !hitPreMouseButton(keyCode);
	}

	/// <summary>
	/// マウスのボタンを離した瞬間を検知
	/// </summary>
	/// <param name="keyCode">Input::MOUSE::なんとか、rgbButtons配列の添え字</param>
	/// <returns>離した瞬間ならtrue</returns>
	inline bool releaseTriggerMouseButton(_In_ BYTE keyCode) {
		return !hitMouseButton(keyCode) && hitPreMouseButton(keyCode);
	}

	MouseMove getMouseMove();

	inline LONG getMouseWheelScroll() { return mouseState.lZ; }

	// @return POINT型(LONG x とLONG y のみの構造体)
	inline const POINT &getMousePos() const { return mousePos; };
	inline DirectX::XMFLOAT2 getMousePosF2() const { return DirectX::XMFLOAT2((float)mousePos.x, (float)mousePos.y); }
	bool setMousePos(int x, int y);

	/// <summary>
	/// マウスカーソルを表示するかどうかを設定
	/// </summary>
	/// <param name="dispFlag">trueで表示、falseで非表示</param>
	void changeDispMouseCursorFlag(bool dispFlag);
};
