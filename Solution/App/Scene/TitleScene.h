/*****************************************************************//**
 * \file   TitleScene.h
 * \brief  タイトル画面
 *********************************************************************/

#pragma once
#include "System/GameScene.h"

#include <memory>
#include <functional>
#include <DirectXMath.h>

#include "2D/DebugText.h"
#include "Input/Input.h"
#include "Sound/Sound.h"

#include "Util/Timer.h"

#include <thread>

 /// @brief タイトル画面シーンのクラス
class TitleScene :
	public GameScene
{
	// --------------------
	// 音
	// --------------------
	std::unique_ptr<Sound> shortBridge;
	std::unique_ptr<Sound> bgm;

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

	// --------------------
	// シーン
	// --------------------
	struct MyThread
	{
		std::unique_ptr<std::thread> thread;

		inline void join()
		{
			if (thread && thread->joinable())
			{
				thread->join();
			}
		}

		~MyThread()
		{
			join();
		}
	};
	std::unique_ptr<MyThread> sceneThread;
	std::unique_ptr<GameScene> nextScene;

	Input* input = nullptr;

	// update_何とか関数を格納する
	std::function<void()> update_proc;

	void update_normal();
	void update_end();

public:
	TitleScene();
	~TitleScene();
	void start() override;
	void update() override;
	void drawFrontSprite() override;
};