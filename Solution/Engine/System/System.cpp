#include "System.h"

#include "WinAPI.h"
#include "Looper.h"

System::System()
{}

void System::update()
{
	if (!error)
	{
		// ゲームループ
		while (!WinAPI::getInstance()->processMessage()
			   && !Looper::getInstance()->loop())
		{
		}
	}
}

System::~System()
{}