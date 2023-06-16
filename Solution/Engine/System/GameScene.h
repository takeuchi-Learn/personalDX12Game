#pragma once
class GameScene
{
public:
	virtual ~GameScene() {}

	virtual void start() {}
	virtual void update() = 0; // 実装必須
	virtual void drawObj3d() {}
	virtual void drawFrontSprite() {}
};
