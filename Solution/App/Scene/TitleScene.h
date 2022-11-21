#pragma once
#include "../Engine/System/GameScene.h"

#include <memory>
#include <functional>
#include <DirectXMath.h>

#include "../Engine/2D/DebugText.h"
#include "../Engine/Input/Input.h"
#include "../Engine/Sound/Sound.h"

class TitleScene :
	public GameScene
{
	// --------------------
	// 音
	// --------------------
	std::unique_ptr<Sound> shortBridge;

	// --------------------
	// スプライト
	// --------------------
	std::unique_ptr<SpriteBase> spCom;
	std::unique_ptr<Sprite> title;
	std::unique_ptr<Sprite> titleBack;

	// --------------------
	// デバッグテキスト
	// --------------------
	std::unique_ptr<DebugText> debugText;
	// デバッグテキスト用のテクスチャ番号
	UINT debugTextTexNumber;

	Input* input = nullptr;

	DirectX::XMFLOAT2 titleStrPos{};

	// update_何とか関数を格納する
	std::function<void()> update_proc;

	void update_end();
	void update_normal();

public:
	TitleScene();
	void update() override;
	void drawFrontSprite() override;
};