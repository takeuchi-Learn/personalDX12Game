#include <Windows.h>

#include "System.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	System* sys = new System();

	sys->update();

	delete sys;
	sys = nullptr;

	return 0;
}