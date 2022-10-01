#include "BossScene.h"
#include "WinAPI.h"
#include "Input.h"
#include "EndScene.h"
#include "SceneManager.h"

#include <DirectXMath.h>
#include <imgui.h>

using namespace DirectX;

BossScene::BossScene() :
	update_proc(std::bind(&BossScene::update_start, this))
{
}

void BossScene::update_start()
{
	update_proc = std::bind(&BossScene::update_play, this);
}

void BossScene::update_play()
{
	if (Input::getInstance()->hitKey(DIK_SPACE))
	{
		update_proc = std::bind(&BossScene::update_end, this);
	}
}

void BossScene::update_end()
{
	SceneManager::getInstange()->changeScene(new EndScene());
}


void BossScene::update()
{
	update_proc();
}

void BossScene::drawObj3d()
{
}

void BossScene::drawFrontSprite()
{
	constexpr ImGuiWindowFlags winFlags =
		// リサイズ不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		// タイトルバー無し
		//ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
		// 設定を.iniに出力しない
		ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings |
		// 移動不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove;
	//// スクロールバーを常に表示
	//ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysHorizontalScrollbar |
	//ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar;

	// 最初のウインドウの位置を指定
	constexpr XMFLOAT2 fstWinPos = XMFLOAT2((float)WinAPI::window_width * 0.02f,
											(float)WinAPI::window_height * 0.02f);

	ImGui::SetNextWindowPos(ImVec2(fstWinPos.x,
								   fstWinPos.y));
	ImGui::SetNextWindowSize(ImVec2(200.f, 100.f));

	ImGui::Begin("ボス戦", nullptr, winFlags);
	ImGui::Text("未実装\nスペースで次のシーンへ進む");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::End();
}
