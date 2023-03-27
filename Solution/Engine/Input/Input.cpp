#include "Input.h"

#include <cmath>

#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "System/WinAPI.h"

Input::Input()
{
	init();
}

Input::~Input()
{}

void Input::init()
{
	HRESULT result = DirectInput8Create(
		WinAPI::getInstance()->getW().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard); // 標準形式

	result = devkeyboard->SetCooperativeLevel(
		WinAPI::getInstance()->getHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

	result = dinput->CreateDevice(GUID_SysMouse, &devmouse, NULL);

	result = devmouse->SetDataFormat(&c_dfDIMouse2); // 標準形式

	result = devmouse->SetCooperativeLevel(WinAPI::getInstance()->getHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

	// PAD
	initPad(padNum);
}

void Input::update()
{
	HRESULT result = devkeyboard->Acquire();

	memcpy(preKey, key, sizeof(key));
	result = devkeyboard->GetDeviceState(sizeof(key), key);

	result = devmouse->Acquire();	// マウス動作開始
	preMouseState = mouseState;
	result = devmouse->GetDeviceState(sizeof(mouseState), &mouseState);

	GetCursorPos(&mousePos);
	ScreenToClient(WinAPI::getInstance()->getHwnd(), &mousePos);

	updatePad(padNum);
}

void Input::resetState()
{
	for (UINT i = 0; i < 256; i++)
	{
		key[i] = 0;
		preKey[i] = 0;
	}

	mouseState = DIMOUSESTATE2();
	preMouseState = DIMOUSESTATE2();

	ZeroMemory(&state, sizeof(XINPUT_STATE));
	ZeroMemory(&preState, sizeof(XINPUT_STATE));
}

bool Input::setMousePos(int x, int y)
{
	POINT tmpPos = { x,y };
	ClientToScreen(WinAPI::getInstance()->getHwnd(), &tmpPos);

	return SetCursorPos(tmpPos.x, tmpPos.y);
}

void Input::changeDispMouseCursorFlag(const bool dispFlag)
{
	ShowCursor((BOOL)dispFlag);
}

bool Input::initPad(DWORD padIndex)
{
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	DWORD dwResult = XInputGetState(0, &state);

	return ERROR_SUCCESS == dwResult;
}

void Input::updatePad(DWORD padIndex)
{
	preState = state;

	initPad(padIndex);

	// 無効な入力は0にする
	if ((state.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
		 state.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
		(state.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
		 state.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
	{
		state.Gamepad.sThumbLX = 0;
		state.Gamepad.sThumbLY = 0;
	}
	if ((state.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		 state.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
		(state.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		 state.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
	{
		state.Gamepad.sThumbRX = 0;
		state.Gamepad.sThumbRY = 0;
	}

	// 左スティックの入力を方向パッドに変換
	state.Gamepad.wButtons |=
		padThumbToDPad(state.Gamepad.sThumbLX,
					   state.Gamepad.sThumbLY,
					   XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
}

WORD Input::padThumbToDPad(SHORT sThumbX, SHORT sThumbY, SHORT sDeadZone)
{
	WORD wButtons = 0;

	if (sThumbY >= sDeadZone)
	{
		wButtons |= XINPUT_GAMEPAD_DPAD_UP;
	} else if (sThumbY <= -sDeadZone)
	{
		wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
	}

	if (sThumbX <= -sDeadZone)
	{
		wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
	} else if (sThumbX >= sDeadZone)
	{
		wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
	}

	return wButtons;
}

DirectX::XMFLOAT2 Input::hitPadLStickRaito() const
{
	const bool isLStickX = this->isVaildPadLStickX();
	const bool isLStickY = this->isVaildPadLStickY();

	DirectX::XMFLOAT2 inputRaito = DirectX::XMFLOAT2(0.f, 0.f);

	if (isLStickX)
	{
		inputRaito.x = (float)hitPadLStickX() / 32767.f;
	}
	if (isLStickY)
	{
		inputRaito.y = (float)hitPadLStickY() / 32767.f;
	}

	return inputRaito;
}

DirectX::XMFLOAT2 Input::hitPadRStickRaito() const
{
	const bool isRStickX = this->isVaildPadRStickX();
	const bool isRStickY = this->isVaildPadRStickY();

	DirectX::XMFLOAT2 inputRaito = DirectX::XMFLOAT2(0.f, 0.f);

	if (isRStickX)
	{
		inputRaito.x = (float)hitPadRStickX() / 32767.f;
	}
	if (isRStickY)
	{
		inputRaito.y = (float)hitPadRStickY() / 32767.f;
	}

	return inputRaito;
}