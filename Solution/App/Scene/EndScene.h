#pragma once
#include "../Engine/System/GameScene.h"

#include <memory>
#include "../../Engine/2D/DebugText.h"
#include "../Engine/Input/Input.h"

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
	// デバッグテキスト用のテクスチャ番号
	UINT debugTextTexNumber;

	Input* input = nullptr;

public:
	EndScene();
	void update() override;
	void drawFrontSprite() override;
};
