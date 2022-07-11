#pragma once
#include "GameScene.h"

#include "DebugText.h"

#include "Input.h"

#include <memory>

class EndScene :
    public GameScene {

	// --------------------
	// デバッグテキスト
	// --------------------
	std::unique_ptr<SpriteBase> spCom;
	std::unique_ptr<DebugText> debugText;
	// デバッグテキスト用のテクスチャ番号
	UINT debugTextTexNumber;

	Input* input = nullptr;

public:
	EndScene();
    void update() override;
    void drawFrontSprite() override;
};

