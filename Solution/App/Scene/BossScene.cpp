#include "BossScene.h"
#include "EndScene.h"

#include <DirectXMath.h>
#include <imgui.h>
#include "../Engine/System/SceneManager.h"
#include "../Engine/System/PostEffect.h"

#include "../Engine/Collision/Collision.h"

using namespace DirectX;

BossScene::BossScene() :
	BaseStage(),

	bossModel(new ObjModel("Resources/tori", "tori")),
	boss(std::make_unique<BossEnemy>(camera.get(), bossModel.get())),

	smallEnemyModel(new ObjModel("Resources/tori", "tori")),

	// スプライト
	bossHpGr(new Sprite(spBase->loadTexture(L"Resources/hpBar.png"), spBase.get(), XMFLOAT2(0.5f, 0.f)))
{
	// --------------------
	// スプライト
	// --------------------
	bossHpGr->position = XMFLOAT3(WinAPI::window_width / 2.f, 0.f, 0.f);
	bossHpGr->setSize(XMFLOAT2(WinAPI::window_width * 0.75f,
							   WinAPI::window_height / 20.f));

	// カメラ
	constexpr float farZ = 1000.f;
	camera->setParentObj(player.get());
	camera->setFarZ(farZ);

	// --------------------
	// ゲームオブジェクト
	// --------------------
	playerHpMax = 20u;
	player->setScale(10.f);
	player->setHp(playerHpMax);

	attackableEnemy.emplace_front(boss.get());
	bossHpMax = 16u;
	boss->setHp(bossHpMax);
	boss->setScale(100.f);
	boss->setPos(XMFLOAT3(0, boss->getScaleF3().y, 300));
	boss->setRotation(XMFLOAT3(0, 180.f, 0));
	boss->setTargetObj(player.get());
	boss->setSmallEnemyModel(bossModel.get());
	boss->changePhase_approach();

	constexpr size_t smallEnemyNum = 3u;
	smallEnemy.resize(smallEnemyNum);
	smallEnemyHpMax = 1u;

	for (UINT i = 0u; i < smallEnemyNum; ++i)
	{
		smallEnemy[i].reset(new BaseEnemy(camera.get(), smallEnemyModel.get()));

		constexpr float enemyScale = 10.f;
		smallEnemy[i]->setScale(enemyScale);
		smallEnemy[i]->setHp(smallEnemyHpMax);
		smallEnemy[i]->setPos(XMFLOAT3(boss->getPos().x + i * enemyScale,
									   smallEnemy[i]->getScaleF3().y,
									   boss->getPos().z));

		attackableEnemy.emplace_front(smallEnemy[i].get());
	}
}

void BossScene::update_start()
{
	PostEffect::getInstance()->setVignIntensity(0.5f);
	PostEffect::getInstance()->setSpeedLineIntensity(0.125f);

	update_proc = std::bind(&BossScene::update_play, this);
}

void BossScene::update_play()
{
	if (Input::getInstance()->hitKey(DIK_SPACE))
	{
		update_proc = std::bind(&BossScene::update_end, this);
	}

	if (input->triggerKey(DIK_P))
	{
		boss->addSmallEnemy();
	}

	updateRgbShift();

	if (player->getAlive())
	{
		// --------------------
		// 自機の移動(と回転)
		// --------------------
		movePlayer();

		// --------------------
		// 照準に敵がいるかどうか
		// --------------------

		const XMFLOAT2 aim2DMin = XMFLOAT2(input->getMousePos().x - aim2D->getSize().x / 2.f,
										   input->getMousePos().y - aim2D->getSize().y / 2.f);
		const XMFLOAT2 aim2DMax = XMFLOAT2(input->getMousePos().x + aim2D->getSize().x / 2.f,
										   input->getMousePos().y + aim2D->getSize().y / 2.f);

		bool targetIsEmpty = true;

		for (BaseEnemy* i : attackableEnemy)
		{
			// いない敵の判定は取らない
			if (!i->getAlive()) { continue; }

			targetIsEmpty = addShotTarget(i, aim2DMin, aim2DMax);
		}
		for (auto& i : boss->getSmallEnemyList())
		{
			if (!i->getAlive()) { continue; }

			targetIsEmpty = addShotTarget(i.get(), aim2DMin, aim2DMax);
		}

		// 照準の中に敵がいれば色を変える
		if (targetIsEmpty)
		{
			aim2D->color = XMFLOAT4(0, 0, 0, 1);
		} else
		{
			aim2D->color = XMFLOAT4(1, 0, 0, 1);
		}

		// --------------------
		// 弾発射
		// --------------------
		if (!player->shotTargetIsEmpty())
		{
			if (input->triggerMouseButton(Input::MOUSE::LEFT))
			{
				constexpr float bulSpeed = 8.f;
				player->shot(camera.get(), playerBulModel.get(), bulSpeed);
			}
		}

		// --------------------
		// 自機弾と敵の当たり判定
		// --------------------
		for (auto& e : attackableEnemy)
		{
			// いない敵は判定しない
			if (!e->getAlive()) { continue; }

			const CollisionShape::Sphere enemy(XMLoadFloat3(&e->calcWorldPos()),
											   e->getScale());

			for (auto& pb : player->getBulArr())
			{
				// 無い弾は判定しない
				if (!pb.getAlive()) { continue; }

				const CollisionShape::Sphere bul(XMLoadFloat3(&pb.calcWorldPos()),
												 pb.getScaleF3().z);

				// 衝突していたら
				if (Collision::CheckHit(bul, enemy))
				{
					// 弾はさよなら
					pb.kill();

					// 敵はダメージを受ける
					// hpが0になったらさよなら
					e->damage(1u, true);
				}
			}
		}

		// --------------------
		// 自機とボス弾(雑魚敵)の当たり判定
		// --------------------
		if (player->getAlive())
		{
			const CollisionShape::Sphere pCol(XMLoadFloat3(&player->getPos()),
											  player->getScale());

			for (auto& i : boss->getSmallEnemyList())
			{
				const CollisionShape::Sphere eCol(XMLoadFloat3(&i->getPos()),
												  i->getScale());

				if (Collision::CheckHit(pCol, eCol))
				{
					// 当たった雑魚敵は消す
					i->kill();

					// 自機がダメージを受ける
					if (player->damage(1u, true))
					{
						// 自機の体力が0になったら
						player->kill();
						update_proc = std::bind(&BossScene::update_end, this);
					} else
					{
						startRgbShift();
					}
				}
			}
		}

		// --------------------
		// 自機弾とボス弾(雑魚敵)の当たり判定
		// --------------------
		for (auto& pb : player->getBulArr())
		{
			if (!pb.getAlive()) { continue; }

			const CollisionShape::Sphere bul(XMLoadFloat3(&pb.calcWorldPos()),
											 pb.getScaleF3().z);

			for (auto& e : boss->getSmallEnemyList())
			{
				if (!e->getAlive()) { continue; }

				const CollisionShape::Sphere enemy(XMLoadFloat3(&e->calcWorldPos()),
												   e->getScale());

				if (Collision::CheckHit(bul, enemy))
				{
					e->damage(1u, true);
					pb.kill();
				}
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

bool BossScene::addShotTarget(BaseEnemy* enemy,
							  const DirectX::XMFLOAT2& aim2DPosMin,
							  const DirectX::XMFLOAT2& aim2DPosMax)
{
	bool targetIsEmpty = true;

	// 敵のスクリーン座標を取得
	const XMFLOAT2 screenEnemyPos = enemy->getObj()->calcScreenPos();

	// 敵が2D照準の中にいるかどうか
	if (aim2DPosMin.x <= screenEnemyPos.x &&
		aim2DPosMin.y <= screenEnemyPos.y &&
		aim2DPosMax.x >= screenEnemyPos.x &&
		aim2DPosMax.y >= screenEnemyPos.y)
	{
		// 敵との距離を更新
		const float nowEnemyDistance = sqrtf(
			powf(enemy->getPos().x - camera->getEye().x, 2.f) +
			powf(enemy->getPos().y - camera->getEye().y, 2.f) +
			powf(enemy->getPos().z - camera->getEye().z, 2.f)
		);
		player->addShotTarget(enemy->getObj());
		targetIsEmpty = false;
	}

	return targetIsEmpty;
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
	{
		XMFLOAT2 size = hpGrSizeMax;
		size.x *= (float)boss->getHp() / (float)bossHpMax;
		bossHpGr->setSize(size);
	}
	bossHpGr->drawWithUpdate(DX12Base::ins(), spBase.get());
	aim2D->drawWithUpdate(DX12Base::ins(), spBase.get());

	ImGui::SetNextWindowPos(ImVec2(fstWinPos.x,
								   fstWinPos.y));
	ImGui::SetNextWindowSize(ImVec2(200.f, 200.f));

	ImGui::Begin("ボス戦", nullptr, winFlags);
	ImGui::Text("スペースで次のシーンへ進む");
	ImGui::Text("WS : 移動");
	ImGui::Text("AD : 回転");
	ImGui::Text("左シフト : ダッシュ");
	ImGui::Text("E : カメラ位置変更");
	ImGui::Text("照準内に敵 + 左クリック : 攻撃");
	ImGui::Text("敵弾数 : %u", boss->calcSmallEnemyNum());
	ImGui::Text("自機体力 : %.2f%%", (float)player->getHp() / (float)playerHpMax * 100.f);
	if (boss->getAlive())
	{
		ImGui::Text("ボスHP : %.2f%% (%u)",
					(float)boss->getHp() / (float)bossHpMax * 100.f,
					boss->getHp());
	}
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::End();
}

BossScene::~BossScene()
{
	PostEffect::getInstance()->setRgbShiftNum({ 0.f, 0.f });

	PostEffect::getInstance()->setSpeedLineIntensity(0.f);
	PostEffect::getInstance()->setVignIntensity(0.25f);

	PostEffect::getInstance()->changePipeLine(0U);
}