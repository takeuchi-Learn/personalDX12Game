#pragma once
class GameScene {
public:
	virtual ~GameScene() {};	// 実装は任意

	virtual void init() {};		// 実装は任意
	virtual void update() = 0;	// 実装必須
	virtual void drawObj3d(){};			// 実装は任意
	virtual void drawFrontSprite(){};	// 実装は任意
};

