#pragma once
#include "GameScene.h"
#include <functional>

class BossScene :
	public GameScene
{
private:
	// update_何とか関数を格納する
	std::function<void()> update_proc;

	void update_start();
	void update_play();
	void update_end();

public:
	BossScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};

