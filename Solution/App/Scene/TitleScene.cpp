#include "TitleScene.h"

#include "PlayScene.h"
#include "RailShoot.h"
#include "BaseStage.h"
#include "../Engine/System/SceneManager.h"
#include <DirectXMath.h>

using namespace DirectX;

TitleScene::TitleScene()
	: titleStrPos(0.f, 0.f),
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
	}
}

void TitleScene::update_end()
{
	titleStrPos.y += 20.f;

	if (titleStrPos.y > WinAPI::window_height)
	{
		SceneManager::getInstange()->changeScene(new RailShoot());
	}

	title->position.y = titleStrPos.y;
}

void TitleScene::drawFrontSprite()
{
	constexpr ImGuiWindowFlags winFlags = DX12Base::imGuiWinFlagsDef |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar;

	// 最初のウインドウの位置とサイズを指定
	constexpr XMFLOAT2 fstWinPos = XMFLOAT2((float)WinAPI::window_width * 0.02f,
											(float)WinAPI::window_height * 0.02f);
	ImGui::SetNextWindowPos(ImVec2(titleStrPos.x + fstWinPos.x,
								   titleStrPos.y + fstWinPos.y));

	constexpr XMFLOAT2 fstWinSize = XMFLOAT2(WinAPI::window_width / 4.f,
											 WinAPI::window_height / 8.f);
	ImGui::SetNextWindowSize(ImVec2(fstWinSize.x,
									fstWinSize.y));

	spCom->drawStart(DX12Base::ins()->getCmdList());
	titleBack->drawWithUpdate(DX12Base::ins(), spCom.get());
	title->drawWithUpdate(DX12Base::ins(), spCom.get());
}