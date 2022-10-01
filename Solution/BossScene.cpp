#include "BossScene.h"
#include "WinAPI.h"
#include "EndScene.h"
#include "SceneManager.h"

#include <DirectXMath.h>
#include <imgui.h>

using namespace DirectX;

BossScene::BossScene() :
	// --------------------
	// シングルトンインスタンス
	// --------------------
	input(Input::getInstance()),

	// --------------------
	// カメラとライト
	// --------------------
	camera(new CameraObj(nullptr)),
	light(new Light()),

	// --------------------
	// モデルとオブジェクト
	// --------------------
	playerModel(new ObjModel("Resources/box", "box")),
	playerBulModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),
	player(new Player(camera.get(), playerModel.get())),

	// --------------------
	// 背景パイプライン
	// --------------------
	backPipelineSet(Object3d::createGraphicsPipeline(Object3d::BLEND_MODE::ALPHA,
													 L"Resources/Shaders/BackVS.hlsl",
													 L"Resources/Shaders/BackPS.hlsl")),

	// --------------------
	// 更新関数
	// --------------------
	update_proc(std::bind(&BossScene::update_start, this))
{
	// カメラ
	constexpr float farZ = 1000.f;
	camera->setParentObj(player.get());
	camera->setFarZ(farZ);

	player->setScale(10.f);

	// 背景オブジェクト
	constexpr float backScale = farZ * 0.9f;
	back.reset(new ObjSet(camera.get(), "Resources/back/", "back", true));
	back->setScale(XMFLOAT3(backScale, backScale, backScale));

	// 地面
	ground.reset(new ObjSet(camera.get(), "Resources/ground", "ground", false));
	constexpr float groundSize = farZ * 2.f;
	ground->setPos(XMFLOAT3(0, -player->getScale(), 0));
	ground->setScale(XMFLOAT3(groundSize, groundSize, groundSize));
	ground->getModelPt()->setTexTilling(XMFLOAT2(groundSize / 32.f, groundSize / 32.f));
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

	// 移動
	{
		const bool hitW = input->hitKey(DIK_W);
		const bool hitS = input->hitKey(DIK_S);

		if (hitW || hitS)
		{
			const float moveSpeed = 90.f / DX12Base::ins()->getFPS();

			if (hitW)
			{
				player->moveForward(moveSpeed);
			} else if (hitS)
			{
				player->moveForward(-moveSpeed);
			}
		}
	}

	// 回転
	{
		const bool hitA = input->hitKey(DIK_A);
		const bool hitD = input->hitKey(DIK_D);

		if (hitA || hitD)
		{
			const float speed = 90.f / DX12Base::ins()->getFPS();

			XMFLOAT3 rota = player->getRotation();

			if (hitA)
			{
				rota.y -= speed;
			} else if (hitD)
			{
				rota.y += speed;
			}

			player->setRotation(rota);
		}
	}
}

void BossScene::update_end()
{
	SceneManager::getInstange()->changeScene(new EndScene());
}


void BossScene::update()
{
	{
		// シーン遷移中も背景は回す
		XMFLOAT2 shiftUv = back->getModelPt()->getShiftUv();
		constexpr float shiftSpeed = 0.1f;

		shiftUv.x += shiftSpeed / DX12Base::getInstance()->getFPS();

		back->getModelPt()->setShivtUv(shiftUv);
	}

	update_proc();

	{
		const XMFLOAT3 camWorldPos = XMFLOAT3(camera->getEye().x,
											  camera->getEye().y,
											  camera->getEye().z);

		light->setLightPos(camWorldPos);
		back->setPos(camWorldPos);
	}
	light->update();
	camera->update();
}

void BossScene::drawObj3d()
{
	Object3d::startDraw(backPipelineSet);
	back->drawWithUpdate(light.get());

	Object3d::startDraw();
	ground->drawWithUpdate(light.get());

	if (player->getAlive())
	{
		player->drawWithUpdate(light.get());
	}
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
