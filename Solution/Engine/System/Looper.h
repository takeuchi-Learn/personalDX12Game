#pragma once
#include "PostEffect.h"
#include <memory>

class Looper
{
private:
	bool exitFlag = false;

	Looper(const Looper& looper) = delete;
	Looper& operator=(const Looper& looper) = delete;

	Looper();

	void loopUpdate();
	void loopDraw();

	void pushImGuiCol();
	void popImGuiCol();

public:
	~Looper();

	inline static Looper* getInstance()
	{
		static std::unique_ptr<Looper> lp(new Looper());
		return lp.get();
	}

	inline void exitGame() { exitFlag = true; }

	// @return 異常の有無(falseで正常)
	bool loop();
};
