#pragma once
#include "../Engine/System/GameScene.h"

#include <memory>
#include <functional>
#include <DirectXMath.h>

#include "../Engine/2D/DebugText.h"
#include "../Engine/Input/Input.h"
#include "../Engine/Sound/Sound.h"

#include "../Engine/Util/Timer.h"

class TitleScene :
	public GameScene
{
	// --------------------
	// 音
	// --------------------
	std::unique_ptr<Sound> shortBridge;

	// --------------------
	// 時間
	// --------------------
	std::unique_ptr<Timer> timer;
	const float sceneChangeTime = Timer::oneSecF;

	// --------------------
	// スプライト
	// --------------------
	std::unique_ptr<SpriteBase> spCom;
	std::unique_ptr<Sprite> titleLogo;
	std::unique_ptr<Sprite> titleOther;
	std::unique_ptr<Sprite> titleBack;

	DirectX::XMFLOAT2 titlePos{};

	// --------------------
	// デバッグテキスト
	// --------------------
	std::unique_ptr<DebugText> debugText;
	// デバッグテキスト用のテクスチャ番号
	UINT debugTextTexNumber;

	Input* input = nullptr;

	// update_何とか関数を格納する
	std::function<void()> update_proc;

	void update_normal();
	void update_end();

public:
	TitleScene();
	void start() override;
	void update() override;
	void drawFrontSprite() override;
};