#pragma once
#include "System/GameScene.h"

#include <memory>
#include "2D/DebugText.h"
#include "Input/Input.h"

class EndScene :
	public GameScene
{
	// --------------------
	// スプライト
	// --------------------
	std::unique_ptr<SpriteBase> spCom;
	std::unique_ptr<Sprite> end;

	// --------------------
	// デバッグテキスト
	// --------------------
	std::unique_ptr<DebugText> debugText;

	Input* input = nullptr;

public:
	EndScene();
	void start() override;
	void update() override;
	void drawFrontSprite() override;
};
