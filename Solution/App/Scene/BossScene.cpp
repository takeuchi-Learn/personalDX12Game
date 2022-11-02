#include "BossScene.h"
#include "EndScene.h"

#include <DirectXMath.h>
#include <imgui.h>
#include "../Engine/System/SceneManager.h"
#include "../Engine/System/PostEffect.h"

using namespace DirectX;

BossScene::BossScene() :
	// --------------------
	// シングルトンインスタンス
	// --------------------
	input(Input::getInstance()),

	timer(new Timer()),

	// --------------------
	// カメラとライト
	// --------------------
	camera(new CameraObj(nullptr)),
	light(new Light()),

	// --------------------
	// モデルとオブジェクト
	// --------------------
	playerModel(new ObjModel("Resources/player", "player")),
	playerBulModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),
	player(new Player(camera.get(), playerModel.get())),

	bossModel(new ObjModel("Resources/tori", "tori")),
	boss(std::make_unique<BaseEnemy>(camera.get(), bossModel.get())),

	// --------------------
	// 背景パイプライン
	// --------------------
	backPipelineSet(Object3d::createGraphicsPipeline(Object3d::BLEND_MODE::ALPHA,
													 L"Resources/Shaders/BackVS.hlsl",
													 L"Resources/Shaders/BackPS.hlsl")),

	// --------------------
	// スプライト
	// --------------------
	spBase(new SpriteBase()),
	aim2D(new Sprite(spBase->loadTexture(L"Resources/aimPos.png"),
					 spBase.get())),

	// --------------------
	// 更新関数
	// --------------------
	update_proc(std::bind(&BossScene::update_start, this))
{
	// カメラ
	constexpr float farZ = 1000.f;
	camera->setParentObj(player.get());
	camera->setFarZ(farZ);

	// ゲームオブジェクト
	player->setScale(10.f);

	boss->setScale(100.f);
	boss->setPos(XMFLOAT3(0, boss->getScaleF3().y, 300));
	boss->setRotation(XMFLOAT3(0, 180.f, 0));
	boss->setPhase([&]
				   {
					   XMVECTOR velVec = XMLoadFloat3(&player->getPos()) - XMLoadFloat3(&boss->getPos());
					   velVec = XMVectorSetY(velVec, 0.f);
					   if (XMVectorGetX(XMVector3Length(velVec)) < boss->getScale())
					   {
						   return;
					   }

					   constexpr float speed = 2.f;
					   velVec = XMVector3Normalize(velVec) * 2.f;

					   XMFLOAT3 vel{ };
					   XMStoreFloat3(&vel, velVec);

					   boss->move(vel);

					   const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(vel);
					   boss->setRotation(XMFLOAT3(rotaDeg.x, rotaDeg.y, 0.f));
				   });

	// 背景オブジェクト
	constexpr float backScale = farZ * 0.9f;
	back.reset(new ObjSet(camera.get(), "Resources/back/", "back", true));
	back->setScale(XMFLOAT3(backScale, backScale, backScale));

	// 地面
	ground.reset(new ObjSet(camera.get(), "Resources/ground", "ground", false));
	constexpr float groundSize = farZ * 2.f;
	ground->setPos(XMFLOAT3(0, -player->getScale(), 0));
	ground->setScale(XMFLOAT3(groundSize, groundSize, groundSize));

	constexpr float tillingNum = groundSize / 32.f;
	ground->getModelPt()->setTexTilling(XMFLOAT2(tillingNum, tillingNum));

	// スプライト読み込み
	aim2D = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/aimPos.png"),
									 spBase.get());
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

	if (Input::getInstance()->triggerKey(DIK_R))
	{
		startRgbShift();
	}
	updateRgbShift();

	// 移動
	{
		const bool hitW = input->hitKey(DIK_W);
		const bool hitS = input->hitKey(DIK_S);

		if (hitW || hitS)
		{
			float moveSpeed = 90.f / DX12Base::ins()->getFPS();
			if (input->hitKey(DIK_LCONTROL))
			{
				moveSpeed /= 2.f;
			} else if (input->hitKey(DIK_LSHIFT))
			{
				moveSpeed *= 2.f;
			}

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
		const bool triggerE = input->triggerKey(DIK_E);

		if (hitA || hitD || triggerE)
		{
			// 上向きか否かの切り替え
			if (triggerE)
			{
				XMFLOAT3 camRrota = camera->getRelativeRotaDeg();

				constexpr float angle = 20.f;
				camRrota.x += playerUpTurn ? angle : -angle;

				playerUpTurn = !playerUpTurn;

				//player->setRotation(rota);
				camera->setRelativeRotaDeg(camRrota);
			}

			// 左右の回転
			if (hitA || hitD)
			{
				float speed = 45.f / DX12Base::ins()->getFPS();

				// 左シフトと左コントロールで速度変更
				if (input->hitKey(DIK_LCONTROL))
				{
					speed /= 2.f;
				} else if (input->hitKey(DIK_LSHIFT))
				{
					speed *= 2.f;
				}

				XMFLOAT3 rota = player->getRotation();

				// 回転させる
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
}

void BossScene::update_end()
{
	SceneManager::getInstange()->changeScene(new EndScene());
}

void BossScene::start()
{
	timer->reset();
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

	{
		aim2D->position.x =
			(float)input->getMousePos().x;
		aim2D->position.y =
			(float)input->getMousePos().y;
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

void BossScene::startRgbShift()
{
	rgbShiftFlag = true;
	nowRgbShiftTime = 0;
	startRgbShiftTime = timer->getNowTime();
}

void BossScene::updateRgbShift()
{
	if (rgbShiftFlag)
	{
		nowRgbShiftTime = timer->getNowTime() - startRgbShiftTime;

		const float raito = (float)nowRgbShiftTime / (float)rgbShiftTimeMax;
		if (raito > 1.f)
		{
			PostEffect::getInstance()->setRgbShiftNum({ 0.f, 0.f });
			rgbShiftFlag = false;
			return;
		}

		// ずらす最大値
		constexpr float rgbShiftMumMax = 1.f / 16.f;

		// イージングを加味した進行割合
		constexpr float  c4 = 2.f * XM_PI / 3.f;
		const float easeRate = -powf(2.f, 10.f * (1.f - raito) - 10.f) *
			DX12Base::ins()->nearSin((raito * 10.f - 10.75f) * c4);

		PostEffect::getInstance()->setRgbShiftNum({ easeRate * rgbShiftMumMax, 0.f });
	}
}

void BossScene::drawObj3d()
{
	Object3d::startDraw(backPipelineSet);
	back->drawWithUpdate(light.get());

	Object3d::startDraw();
	ground->drawWithUpdate(light.get());

	boss->drawWithUpdate(light.get());

	if (player->getAlive())
	{
		player->drawWithUpdate(light.get());
	}
}

void BossScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());
	aim2D->drawWithUpdate(DX12Base::ins(), spBase.get());

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
	ImGui::SetNextWindowSize(ImVec2(200.f, 200.f));

	ImGui::Begin("ボス戦", nullptr, winFlags);
	ImGui::Text("未実装\nスペースで次のシーンへ進む");
	ImGui::Text("WS : 移動");
	ImGui::Text("AD : 回転");
	ImGui::Text("左シフト : ダッシュ");
	ImGui::Text("E : カメラ位置変更");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::End();
}