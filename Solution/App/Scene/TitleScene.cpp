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

	spCom.reset(new SpriteBase());

	// デバッグテキスト用のテクスチャ読み込み
	debugTextTexNumber = spCom->loadTexture(L"Resources/debugfont.png");

	debugText.reset(new DebugText(debugTextTexNumber, spCom.get()));
}

void TitleScene::update()
{
	update_proc();
	debugText->Print(spCom.get(), "3D Shooting", titleStrPos.x, titleStrPos.y, 5.f);
	debugText->Print(spCom.get(), "Press SPACE...", titleStrPos.x, titleStrPos.y + WinAPI::window_height / 2.f, 1.f);
}

void TitleScene::update_normal()
{
	if (input->triggerKey(DIK_SPACE))
	{
		update_proc = std::bind(&TitleScene::update_end, this);
	}
}

void TitleScene::update_end()
{
	titleStrPos.y += 20.f;

	if (titleStrPos.y > WinAPI::window_height)
	{
		SceneManager::getInstange()->changeScene(new RailShoot());
	}
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

	ImGui::Begin("情報", nullptr, winFlags);
	ImGui::PushFont(DX12Base::ins()->getBigImFont());
	ImGui::Text("げぇむたいとるっ！");
	ImGui::PopFont();
	ImGui::Text("Press Space");
	ImGui::End();
}