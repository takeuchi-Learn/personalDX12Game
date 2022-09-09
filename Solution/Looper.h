#pragma once
#include "PostEffect.h"
#include <memory>

class Looper
{
private:
	Looper(const Looper& looper) = delete;
	Looper& operator=(const Looper& looper) = delete;

	Looper();

	// @return 異常の有無(異常があればtrue)
	bool loopUpdate();
	// @return 異常の有無(異常があればtrue)
	bool loopDraw();

public:
	~Looper();

	inline static Looper* getInstance()
	{
		static std::unique_ptr<Looper> lp(new Looper());
		return lp.get();
	}

	// @return 異常の有無(falseで正常)
	bool loop();
};
