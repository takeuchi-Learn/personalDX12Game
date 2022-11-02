#include "BossScene.h"
#include "EndScene.h"

#include <DirectXMath.h>
#include <imgui.h>
#include "../Engine/System/SceneManager.h"
#include "../Engine/System/PostEffect.h"

using namespace DirectX;

BossScene::BossScene() :
	BaseStage(),

	bossModel(new ObjModel("Resources/tori", "tori")),
	boss(std::make_unique<BaseEnemy>(camera.get(), bossModel.get())),

	smallEnemyModel(new ObjModel("Resources/tori", "tori"))
{
	// カメラ
	constexpr float farZ = 1000.f;
	camera->setParentObj(player.get());
	camera->setFarZ(farZ);

	// ゲームオブジェクト
	player->setScale(10.f);

	attackableEnemy.emplace_front(boss.get());
	boss->setScale(100.f);
	boss->setPos(XMFLOAT3(0, boss->getScaleF3().y, 300));
	boss->setRotation(XMFLOAT3(0, 180.f, 0));
	{
		boss->setPhase([&]
					   {
						   // ボスから自機へ向かうベクトル
						   XMVECTOR velVec = XMLoadFloat3(&player->getPos()) - XMLoadFloat3(&boss->getPos());

						   // Y方向には移動しない
						   velVec = XMVectorSetY(velVec, 0.f);

						   // 一定距離より近ければ移動しない
						   if (XMVectorGetX(XMVector3Length(velVec)) < boss->getScaleF3().x)
						   {
							   return;
						   }

						   // 大きさを反映
						   constexpr float speed = 2.f;
						   velVec = XMVector3Normalize(velVec) * 2.f;

						   // XMFLOAT3に変換
						   XMFLOAT3 vel{ };
						   XMStoreFloat3(&vel, velVec);

						   // 移動
						   boss->move(vel);

						   // 速度に合わせて回転
						   const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(vel);
						   boss->setRotation(XMFLOAT3(rotaDeg.x, rotaDeg.y, 0.f));
					   });
	}

	constexpr size_t smallEnemyNum = 1u;
	smallEnemy.resize(smallEnemyNum);

	for (auto& i : smallEnemy)
	{
		i.reset(new BaseEnemy(camera.get(), smallEnemyModel.get()));

		i->setScale(10.f);
		i->setPos(XMFLOAT3(boss->getPos().x, i->getScaleF3().y, boss->getPos().z));

		attackableEnemy.emplace_front(i.get());
	}
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

	updateRgbShift();

	// 自機の移動(と回転)
	movePlayer();

	// 当たり判定
	if (player->getAlive())
	{
		const XMFLOAT2 aim2DMin = XMFLOAT2(input->getMousePos().x - aim2D->getSize().x / 2.f,
										   input->getMousePos().y - aim2D->getSize().y / 2.f);
		const XMFLOAT2 aim2DMax = XMFLOAT2(input->getMousePos().x + aim2D->getSize().x / 2.f,
										   input->getMousePos().y + aim2D->getSize().y / 2.f);

		XMFLOAT2 screenEnemyPos{};
		// 遠い敵を調べるためのもの
		float nowEnemyDistance{};
		BaseEnemy* farthestEnemyPt = nullptr;
		float farthestEnemyLen = 1.f;

		for (BaseEnemy* i : attackableEnemy)
		{
			// いない敵の判定は取らない
			if (!i->getAlive()) { continue; }

			// 敵のスクリーン座標を取得
			screenEnemyPos = i->getObj()->calcScreenPos();

			// 敵が2D照準の中にいるかどうか
			if (aim2DMin.x <= screenEnemyPos.x &&
				aim2DMin.y <= screenEnemyPos.y &&
				aim2DMax.x >= screenEnemyPos.x &&
				aim2DMax.y >= screenEnemyPos.y)
			{
				// 敵との距離を更新
				nowEnemyDistance = sqrtf(
					powf(i->getPos().x - camera->getEye().x, 2.f) +
					powf(i->getPos().y - camera->getEye().y, 2.f) +
					powf(i->getPos().z - camera->getEye().z, 2.f)
				);
				// 照準の中で最も遠い敵なら情報を取っておく
				if (farthestEnemyLen < nowEnemyDistance)
				{
					farthestEnemyPt = i;
					farthestEnemyLen = nowEnemyDistance;
				}
			}
		}
		// 照準の中に敵がいればそこへ弾を出す
		// いなければターゲットはいない
		if (farthestEnemyPt != nullptr)
		{
			player->setShotTarget(farthestEnemyPt->getObj());
			aim2D->color = XMFLOAT4(1, 0, 0, 1);
		} else
		{
			player->setShotTarget(nullptr);
			aim2D->color = XMFLOAT4(0, 0, 0, 1);
		}

		// --------------------
		// 弾発射
		// --------------------
		if (player->shotTargetIsEmpty())
		{
			if (input->triggerMouseButton(Input::MOUSE::LEFT))
			{
				constexpr float bulSpeed = 8.f;
				player->shot(camera.get(), playerBulModel.get(), bulSpeed);
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

void BossScene::movePlayer()
{
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

void BossScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());
	aim2D->drawWithUpdate(DX12Base::ins(), spBase.get());

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