#pragma once

#include <Windows.h>

#include <wrl.h> //ComPtr

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#include <DirectXMath.h>

#include <Xinput.h>

class Input
{
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

#pragma region 共通

private:
	Input(const Input& ip) = delete;
	Input& operator=(const Input& ip) = delete;

	Input();

	ComPtr<IDirectInput8> dinput;

public:
	inline static Input* getInstance()
	{
		static Input input{};
		return &input;
	}
	inline static Input* ins()
	{
		return getInstance();
	}

	void init();
	void update();
	void resetState();

#pragma endregion 共通

#pragma region キーボード

private:
	~Input();

	BYTE key[256];
	BYTE preKey[256];

public:

	inline bool hitKey(BYTE keyCode) const { return (bool)key[keyCode]; }
	inline bool hitPreKey(BYTE keyCode) const { return (bool)preKey[keyCode]; }

	inline bool triggerKey(BYTE keyCode) const { return (bool)(key[keyCode] && preKey[keyCode] == false); }

#pragma endregion キーボード

#pragma region マウス

private:

	DIMOUSESTATE2 mouseState{};
	DIMOUSESTATE2 preMouseState{};

	POINT mousePos{};

	ComPtr<IDirectInputDevice8> devkeyboard;
	ComPtr<IDirectInputDevice8> devmouse;

public:

	struct MouseMove
	{
		LONG x;
		LONG y;
		LONG wheel;
	};

	enum MOUSE : BYTE
	{
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
	inline bool hitMouseButton(_In_ BYTE keyCode)
	{
		return (bool)mouseState.rgbButtons[keyCode];
	}

	// @param keyCode Input::MOUSE::なんとか、rgbButtons配列の添え
	inline bool hitPreMouseButton(_In_ BYTE keyCode)
	{
		return (bool)preMouseState.rgbButtons[keyCode];
	}

	/// @brief マウスのボタンを押した瞬間を検知
	/// @param keyCode Input::MOUSE::なんとか、rgbButtons配列の添え字
	/// @return 押した瞬間ならtrue
	inline bool triggerMouseButton(_In_ BYTE keyCode)
	{
		return hitMouseButton(keyCode) && !hitPreMouseButton(keyCode);
	}

	/// @brief マウスのボタンを離した瞬間を検知
	/// @param keyCode Input::MOUSE::なんとか、rgbButtons配列の添え字
	/// @return 離した瞬間ならtrue
	inline bool releaseTriggerMouseButton(_In_ BYTE keyCode)
	{
		return !hitMouseButton(keyCode) && hitPreMouseButton(keyCode);
	}

	inline MouseMove getMouseMove()
	{
		return MouseMove
		{
			.x = mouseState.lX,
			.y = mouseState.lY,
			.wheel = mouseState.lZ
		};
	}

	inline LONG getMouseWheelScroll() { return mouseState.lZ; }

	// @return POINT型(LONG x とLONG y のみの構造体)
	inline const POINT& getMousePos() const { return mousePos; };
	inline DirectX::XMFLOAT2 getMousePosF2() const { return DirectX::XMFLOAT2((float)mousePos.x, (float)mousePos.y); }
	bool setMousePos(int x, int y);

	/// @brief マウスカーソルを表示するかどうかを設定
	/// @param dispFlag trueで表示、falseで非表示
	void changeDispMouseCursorFlag(bool dispFlag);

#pragma endregion マウス

#pragma region パッドXInput

public:
	/// @brief ゲームパッドの列挙体
	class PAD
	{
	public:
		/// @brief 列挙型に使う型
		using enumType = WORD;

		/// @brief 実際使われる列挙型
		enum PADNUM : enumType
		{
			A = (enumType)XINPUT_GAMEPAD_A,
			B = (enumType)XINPUT_GAMEPAD_B,
			X = (enumType)XINPUT_GAMEPAD_X,
			Y = (enumType)XINPUT_GAMEPAD_Y,
			LB = (enumType)XINPUT_GAMEPAD_LEFT_SHOULDER,
			RB = (enumType)XINPUT_GAMEPAD_RIGHT_SHOULDER,
			UP = (enumType)XINPUT_GAMEPAD_DPAD_UP,
			DOWN = (enumType)XINPUT_GAMEPAD_DPAD_DOWN,
			LEFT = (enumType)XINPUT_GAMEPAD_DPAD_LEFT,
			RIGHT = (enumType)XINPUT_GAMEPAD_DPAD_RIGHT,
			LEFT_THUMB = (enumType)XINPUT_GAMEPAD_LEFT_THUMB,
			RIGHT_THUMB = (enumType)XINPUT_GAMEPAD_RIGHT_THUMB,
			START = (enumType)XINPUT_GAMEPAD_START,
			BACK = (enumType)XINPUT_GAMEPAD_BACK,
		};

	private:
		/// @brief 列挙体の中身を格納
		PADNUM num;

	public:
		/// @brief 列挙体本体の値を取得
		/// @return 列挙体本体の値
		inline PADNUM get() const { return num; }

		/// @brief 列挙型の値で初期化するコンストラクタ
		PAD(PADNUM num) : num(num) {}

		/// @brief 引数無しならAで初期化
		PAD() : num(PADNUM::A) {}

		/// @brief デストラクタ(何もしない)
		~PAD() {}

		/// @brief 整数型への変換
		operator enumType() { return num; }
	};

private:
	XINPUT_STATE state;
	XINPUT_STATE preState;

	DWORD padNum = 0;

public:
	bool initPad(DWORD padIndex);

	void updatePad(DWORD padIndex);

	/// @brief スティック入力を方向パッドフラグとして取得
	/// @param sThumbX スティックのX
	/// @param sThumbY スティックのY
	/// @param sDeadZone デッドゾーン
	/// @return 方向パッドフラグ
	WORD padThumbToDPad(SHORT sThumbX, SHORT sThumbY, SHORT sDeadZone);

	inline bool hitPadButton(WORD button) const
	{
		return state.Gamepad.wButtons & button;
	}
	inline bool triggerPadButton(WORD button) const
	{
		return (state.Gamepad.wButtons & button) &&
			!(preState.Gamepad.wButtons & button);
	}
	inline bool releaseTriggerPadButton(WORD button) const
	{
		return !(state.Gamepad.wButtons & button) &&
			(preState.Gamepad.wButtons & button);
	}

#pragma region パッドのトリガーボタン

	inline BYTE hitPadLTVal() const { return state.Gamepad.bLeftTrigger; }
	inline BYTE hitPadRTVal() const { return state.Gamepad.bRightTrigger; }
	inline bool hitInputPadLT() const
	{
		return state.Gamepad.bLeftTrigger >
			XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	}
	inline bool hitInputPadRT() const
	{
		return state.Gamepad.bRightTrigger >
			XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	}

#pragma endregion パッドのトリガーボタン

#pragma region 左スティック

	inline SHORT hitPadLStickY() const
	{
		return state.Gamepad.sThumbLY;
	}
	inline SHORT hitPadLStickX() const
	{
		return state.Gamepad.sThumbLX;
	}

	inline bool hitPadLStickUp() const
	{
		return state.Gamepad.sThumbLY >=
			XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	}
	inline bool hitPadLStickDown() const
	{
		return state.Gamepad.sThumbLY <=
			-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	}

	inline bool isVaildPadLStickY() const
	{
		return hitPadLStickUp() || hitPadLStickDown();
	}

	inline bool hitPadLStickLeft() const
	{
		return state.Gamepad.sThumbLX <=
			-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	}
	inline bool hitPadLStickRight() const
	{
		return state.Gamepad.sThumbLX >=
			XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	}
	inline bool isVaildPadLStickX() const
	{
		return hitPadLStickLeft() || hitPadLStickRight();
	}

	DirectX::XMFLOAT2 hitPadLStickRaito() const;

#pragma endregion 左スティック

#pragma region 右スティック

	inline SHORT hitPadRStickX() const
	{
		return state.Gamepad.sThumbRX;
	}
	inline SHORT hitPadRStickY() const
	{
		return state.Gamepad.sThumbRY;
	}

	inline bool hitPadRStickUp() const
	{
		return state.Gamepad.sThumbRY >=
			XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	}
	inline bool hitPadRStickDown() const
	{
		return state.Gamepad.sThumbRY <=
			-XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	}

	inline bool isVaildPadRStickY() const
	{
		return hitPadRStickUp() || hitPadRStickDown();
	}

	inline bool hitPadRStickLeft() const
	{
		return state.Gamepad.sThumbRX <=
			-XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	}
	inline bool hitPadRStickRight() const
	{
		return state.Gamepad.sThumbRX >=
			XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	}
	inline bool isVaildPadRStickX() const
	{
		return hitPadRStickLeft() || hitPadRStickRight();
	}

	DirectX::XMFLOAT2 hitPadRStickRaito() const;

#pragma endregion 右スティック

#pragma endregion パッドXInput
};
