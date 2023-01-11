﻿#include "TitleScene.h"

#include "RailShoot.h"
#include "BaseStage.h"
#include "System/SceneManager.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace
{
	inline float easeOutBounce(float t)
	{
		constexpr float n1 = 7.5625f;
		constexpr float d1 = 2.75f;

		if (t < 1.f / d1) { return n1 * t * t; }
		if (t < 2.f / d1) { return n1 * (t -= 1.5f / d1) * t + 0.75f; }
		if (t < 2.5f / d1) { return n1 * (t -= 2.25f / d1) * t + 0.9375f; }
		return n1 * (t -= 2.625f / d1) * t + 0.984375f;
	}
}

TitleScene::TitleScene()
	: timer(std::make_unique<Timer>()),
	update_proc(std::bind(&TitleScene::update_normal, this))
{
	input = Input::getInstance();

	shortBridge = std::make_unique<Sound>("Resources/SE/Shortbridge29-1.wav");

	spCom.reset(new SpriteBase());

	// デバッグテキスト用のテクスチャ読み込み
	debugText.reset(new DebugText(spCom->loadTexture(L"Resources/debugfont.png"),
								  spCom.get()));

	titlePos = XMFLOAT2(0.f, 0.f);

	titleLogo = std::make_unique<Sprite>(spCom->loadTexture(L"Resources/title_logo.png"),
										 spCom.get(),
										 titlePos);
	titleLogo->setSize(XMFLOAT2((float)WinAPI::window_width, (float)WinAPI::window_height));

	titleOther = std::make_unique<Sprite>(spCom->loadTexture(L"Resources/title_other.png"),
										  spCom.get(),
										  titlePos);
	titleOther->setSize(XMFLOAT2((float)WinAPI::window_width, (float)WinAPI::window_height));

	titleBack = std::make_unique<Sprite>(spCom->loadTexture(L"Resources/titleBack.png"),
										 spCom.get(),
										 XMFLOAT2(0.f, 0.f));
	titleBack->setSize(XMFLOAT2((float)WinAPI::window_width, (float)WinAPI::window_height));
}

void TitleScene::start()
{
	timer->reset();
}

void TitleScene::update()
{
	update_proc();
}

void TitleScene::update_normal()
{
	constexpr float logoMoveRange = 16.f;
	titleLogo->position.y = logoMoveRange * std::sin((float)timer->getNowTime() / Timer::oneSecF);

	if (input->triggerKey(DIK_SPACE) ||
		input->getPadButton(Input::PAD::A) ||
		input->getPadButton(Input::PAD::B))
	{
		update_proc = std::bind(&TitleScene::update_end, this);
		Sound::SoundPlayWave(shortBridge.get());
		timer->reset();
	}
}

void TitleScene::update_end()
{
	const float raito = (float)timer->getNowTime() / sceneChangeTime;
	if (raito >= 1.f)
	{
		titlePos.y = static_cast<float>(WinAPI::window_height + 1);
		SceneManager::getInstange()->changeScene<RailShoot>();
	} else
	{
		titlePos.y = std::lerp(0.f,
							   (float)WinAPI::window_height,
							   easeOutBounce(raito));
	}
	titleLogo->position.y = titlePos.y;
	titleOther->position.y = titlePos.y;
}

void TitleScene::drawFrontSprite()
{
	spCom->drawStart(DX12Base::ins()->getCmdList());
	titleBack->drawWithUpdate(DX12Base::ins(), spCom.get());
	titleOther->drawWithUpdate(DX12Base::ins(), spCom.get());
	titleLogo->drawWithUpdate(DX12Base::ins(), spCom.get());
}