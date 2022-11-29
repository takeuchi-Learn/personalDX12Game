#include "TitleScene.h"

#include "PlayScene.h"
#include "RailShoot.h"
#include "BaseStage.h"
#include "../Engine/System/SceneManager.h"
#include <DirectXMath.h>

using namespace DirectX;

TitleScene::TitleScene()
	: timer(std::make_unique<Timer>()),
	update_proc(std::bind(&TitleScene::update_normal, this))
{
	input = Input::getInstance();

	shortBridge = std::make_unique<Sound>("Resources/SE/Shortbridge29-1.wav");

	spCom.reset(new SpriteBase());

	// デバッグテキスト用のテクスチャ読み込み
	debugTextTexNumber = spCom->loadTexture(L"Resources/debugfont.png");

	debugText.reset(new DebugText(debugTextTexNumber, spCom.get()));

	title = std::make_unique<Sprite>(spCom->loadTexture(L"Resources/title.png"),
									 spCom.get(),
									 XMFLOAT2(0.f, 0.f));
	titleBack = std::make_unique<Sprite>(spCom->loadTexture(L"Resources/titleBack.png"),
										 spCom.get(),
										 XMFLOAT2(0.f, 0.f));
}

void TitleScene::start() {
	timer->reset();
}

void TitleScene::update()
{
	update_proc();
}

void TitleScene::update_normal()
{
	if (input->triggerKey(DIK_SPACE))
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
		title->position.y = (float)WinAPI::window_height;
		SceneManager::getInstange()->changeScene(new RailShoot());
		return;
	}

	title->position.y = std::lerp(0.f,
								  (float)WinAPI::window_height,
								  1.f - pow(1.f - raito, 4.f));
}

void TitleScene::drawFrontSprite()
{
	spCom->drawStart(DX12Base::ins()->getCmdList());
	titleBack->drawWithUpdate(DX12Base::ins(), spCom.get());
	title->drawWithUpdate(DX12Base::ins(), spCom.get());
}