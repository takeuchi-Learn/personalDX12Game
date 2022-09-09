#include "Input.h"

#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "WinAPI.h"

Input::Input()
{
	init();
}

Input::~Input()
{
}

Input* Input::getInstance()
{
	static Input ip;
	return &ip;
}

void Input::init()
{
	HRESULT result;

	result = DirectInput8Create(
		WinAPI::getInstance()->getW().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard); // 標準形式

	result = devkeyboard->SetCooperativeLevel(
		WinAPI::getInstance()->getHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

	result = dinput->CreateDevice(GUID_SysMouse, &devmouse, NULL);

	result = devmouse->SetDataFormat(&c_dfDIMouse2); // 標準形式

	result = devmouse->SetCooperativeLevel(WinAPI::getInstance()->getHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
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
}

bool Input::hitKey(BYTE keyCode) { return (bool)key[keyCode]; }
bool Input::hitPreKey(BYTE keyCode) { return (bool)preKey[keyCode]; }

bool Input::triggerKey(BYTE keyCode) { return (bool)(key[keyCode] && preKey[keyCode] == false); }

void Input::resetState()
{
	for (UINT i = 0; i < 256; i++)
	{
		key[i] = 0;
		preKey[i] = 0;

		mouseState = DIMOUSESTATE2();
		preMouseState = DIMOUSESTATE2();
	}
}

Input::MouseMove Input::getMouseMove()
{
	MouseMove ret{
		.x = mouseState.lX,
		.y = mouseState.lY,
		.wheel = mouseState.lZ
	};
	return ret;
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