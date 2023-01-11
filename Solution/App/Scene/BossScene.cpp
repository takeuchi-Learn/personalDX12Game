#include "BossScene.h"
#include "EndScene.h"

#include <DirectXMath.h>
#include <imgui.h>
#include "System/SceneManager.h"
#include "System/PostEffect.h"
#include <Util/RandomNum.h>

#include "Collision/Collision.h"

using namespace DirectX;

namespace
{
	inline XMFLOAT3 lerp(const XMFLOAT3& s, const XMFLOAT3& e, float t)
	{
		return XMFLOAT3(
			std::lerp(s.x, e.x, t),
			std::lerp(s.y, e.y, t),
			std::lerp(s.z, e.z, t)
		);
	}
}

#pragma region 初期化

BossScene::BossScene() :
	BaseStage(),
	playerHpBarWidMax(WinAPI::window_width * 0.25f)
{
	initSprite();

	// カメラ
	camera->setParentObj(player.get());
	sceneChangeStartCamLen = camera->getEye2TargetLen() * 10.f;
	sceneChangeEndCamLen = camera->getEye2TargetLen();

	initGameObj();
}

void BossScene::initSprite()
{
	const UINT hpBarTex = spBase->loadTexture(L"Resources/hpBar.png");

	// ボス体力
	bossHpGr = std::make_unique<Sprite>(hpBarTex, spBase.get(), XMFLOAT2(0.5f, 0.f));
	bossHpGr->setSize(XMFLOAT2(0.f, hpGrSizeMax.y));
	bossHpGr->position = XMFLOAT3(WinAPI::window_width / 2.f, bossHpGr->getSize().y / 2.f, 0.f);
	bossHpGr->color = XMFLOAT4(1.f, 0.5f, 0.f, 1.f);

	// 自機体力
	playerHpBar = std::make_unique<Sprite>(hpBarTex, spBase.get(), XMFLOAT2(0.f, 1.f));
	playerHpBar->setSize(XMFLOAT2(0.f, (float)WinAPI::window_height / 32.f));
	playerHpBar->position = XMFLOAT3(WinAPI::window_width / 20.f, WinAPI::window_height - playerHpBar->getSize().y, 0.f);
	playerHpBar->color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f);

	playerHpBarEdge = std::make_unique<Sprite>(hpBarTex, spBase.get(), XMFLOAT2(0.f, 1.f));
	playerHpBarEdge->setSize(XMFLOAT2(playerHpBarWidMax, playerHpBar->getSize().y));
	playerHpBarEdge->position = playerHpBar->position;
}

void BossScene::initGameObj()
{
	bossModel = std::make_unique<ObjModel>("Resources/tori", "tori");
	boss = std::make_unique<BossEnemy>(camera.get(), bossModel.get());

	initPlayer();

	initEnemy();
}

void BossScene::initPlayer()
{
	playerHpMax = 20u;
	player->setScale(10.f);
	player->setHp(playerHpMax);

	player->setBulLife(600ui16);

	sceneChangeStartPos = XMFLOAT3(0.f, 500.f, 0.f);
	sceneChangeEndPos = XMFLOAT3(0.f, 0.f, 0.f);

	sceneChangeStartRota = player->getRotation();
	sceneChangeStartRota.y += 90.f;
	sceneChangeEndRota = player->getRotation();
}

void BossScene::initEnemy()
{
	// ボスの初期化
	initBoss();
}

void BossScene::initBoss()
{
	boss->setScale(100.f);
	boss->setPos(XMFLOAT3(0, boss->getScaleF3().y, 300));
	boss->setRotation(XMFLOAT3(0, 180.f, 0));
	boss->setTargetObj(player.get());
	boss->setSmallEnemyModel(bossModel.get());
	boss->getObj()->color = XMFLOAT4(4, 4, 4, 1);
	boss->setAlive(false);

	// ボスのパーツ
	bossPartsModel = std::make_unique<ObjModel>("Resources/koshi", "koshi", 0U, false);
	bossParts.resize(8);
	for (auto& i : bossParts)
	{
		i = std::make_unique<BaseEnemy>(camera.get(), bossPartsModel.get());

		// ボス本体を親とする
		i->setParent(boss->getObj());

		// 大きさを変更
		constexpr float scale = 0.1f;
		i->setScale(scale);

		// 体力を設定
		constexpr uint16_t hp = 10ui16;
		i->setHp(hp);
		bossHpMax += hp;

		// 攻撃可能な敵リストに追加
		attackableEnemy.emplace_front(i.get());
	}
	XMFLOAT3 pos{};

	// 肩
	pos = bossParts[1]->getPos();
	pos.x += 0.25f;
	pos.y += 0.5f;
	bossParts[1]->setPos(pos);

	pos = bossParts[2]->getPos();
	pos.x -= 0.25f;
	pos.y += 0.5f;
	bossParts[2]->setPos(pos);

	// 足
	pos = bossParts[3]->getPos();
	pos.x += 0.25f;
	pos.y -= 0.75f;
	bossParts[3]->setPos(pos);


	pos = bossParts[4]->getPos();
	pos.x -= 0.25f;
	pos.y -= 0.75f;
	bossParts[4]->setPos(pos);

	// 脚
	pos = bossParts[5]->getPos();
	pos.x += 0.25f;
	pos.y -= 0.25f;
	pos.z -= 0.125f;
	bossParts[5]->setPos(pos);

	pos = bossParts[6]->getPos();
	pos.x -= 0.25f;
	pos.y -= 0.25f;
	pos.z -= 0.125f;
	bossParts[6]->setPos(pos);

	// 頭
	pos = bossParts[7]->getPos();
	pos.y += 0.875f;
	pos.z += 0.5f;
	bossParts[7]->setPos(pos);
}

void BossScene::start()
{
	PostEffect::getInstance()->setVignIntensity(0.5f);
	PostEffect::getInstance()->setSpeedLineIntensity(0.125f);

	for (auto& i : attackableEnemy)
	{
		i->setAlive(false);
	}

	timer->reset();
}

#pragma endregion 初期化

void BossScene::update_start()
{
	if (timer->getNowTime() > sceneChangeTime)
	{
		player->setPos(sceneChangeEndPos);
		player->setRotation(sceneChangeEndRota);
		camera->setEye2TargetLen(sceneChangeEndCamLen);

		startAppearBoss();
		return;
	}

	const float raito = (float)timer->getNowTime() / (float)sceneChangeTime;

	player->setPos(lerp(sceneChangeStartPos, sceneChangeEndPos, raito));
	player->setRotation(lerp(sceneChangeStartRota, sceneChangeEndRota, raito));

	const float camLen = std::lerp(sceneChangeStartCamLen, sceneChangeEndCamLen, raito);
	camera->setEye2TargetLen(camLen);
}

void BossScene::update_appearBoss()
{
	// 時間が来たら次へ進む
	if (timer->getNowTime() > appearBossData->appearBossTime)
	{
		endAppearBoss();
		return;
	}

	// 進行度[0~1]
	const float raito = (float)timer->getNowTime() / appearBossData->appearBossTime;

	// カメラ回転
	XMFLOAT3 rota = camera->getRelativeRotaDeg();
	rota.y = std::lerp(appearBossData->startCamAngle,
					   appearBossData->endCamAngle,
					   raito);
	camera->setRelativeRotaDeg(rota);

	// イージング(四乗)
	const float camRaito = raito * raito * raito * raito;

	// カメラと追従対象のの距離
	const float camLen = std::lerp(appearBossData->startCamLen,
								   appearBossData->endCamLen,
								   camRaito);
	camera->setEye2TargetLen(camLen);

	// 体力バー
	float barRaito = 1.f - raito;
	barRaito *= barRaito * barRaito * barRaito;
	barRaito = 1.f - barRaito;

	float scaleRaito = std::lerp(appearBossData->startBossHpGrScale,
								 appearBossData->endBossHpGrScale,
								 barRaito);
	bossHpGr->setSize(XMFLOAT2(scaleRaito, bossHpGr->getSize().y));

	scaleRaito = std::lerp(appearBossData->startPlayerHpGrScale,
						   appearBossData->endPlayerHpGrScale,
						   barRaito);
	playerHpBar->setSize(XMFLOAT2(scaleRaito, playerHpBar->getSize().y));
}

void BossScene::update_play()
{
#ifdef _DEBUG

	if (Input::getInstance()->hitKey(DIK_SPACE))
	{
		update_proc = std::bind(&BossScene::update_end, this);
	}

#endif // _DEBUG

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

		std::forward_list<BaseEnemy*> inAim2DEnemy = attackableEnemy;
		for (auto& i : boss->getSmallEnemyList())
		{
			inAim2DEnemy.emplace_front(i.get());
		}
		addShotTarget(inAim2DEnemy, aim2DMin, aim2DMax);

		// --------------------
		// 弾発射
		// --------------------
		if (player->getShotTarget())
		{
			if (input->triggerMouseButton(Input::MOUSE::LEFT) ||
				input->triggerPadButton(Input::PAD::RB) ||
				input->triggerPadButton(Input::PAD::A) ||
				input->triggerPadButton(Input::PAD::B))
			{
				constexpr float bulSpeed = 2.f;
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
					if (e->damage(1u, true))
					{
						e->setDrawFlag(false);
						// 赤エフェクトを出す
						createParticle(e->calcWorldPos(), 128U, 16.f, 16.f, XMFLOAT3(1.f, 0.25f, 0.25f));
					} else
					{
						// シアンエフェクトを出す
						createParticle(e->calcWorldPos(), 96U, 12.f, 12.f, XMFLOAT3(0.25f, 1.f, 1.f));
					}
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
				// いなければ判定しない
				if (!i->getAlive()) { continue; }

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

					// エフェクトを出す
					createParticle(e->calcWorldPos(), 16U, 8.f, 4.f, XMFLOAT3(1.f, 0.25f, 0.25f));
				}
			}
		}

		if (boss->getAlive())
		{
			bool alive = false;
			for (auto& i : bossParts)
			{
				//todo bossPartsが全て死んでいたらボスは死ぬ
				if (i->getAlive())
				{
					alive = true;
				}
			}
			if (!alive)
			{
				boss->kill();
			}
		}

		if (!boss->getAlive())
		{
			startKillBoss();
		}
	}

	// ボス体力バーの大きさを変更
	float oldLen = bossHpGr->getSize().x;
	float nowLen = (float)calcBossHp() / (float)bossHpMax * hpGrSizeMax.x;
	bossHpGr->setSize(XMFLOAT2(std::lerp(oldLen, nowLen, 0.5f), hpGrSizeMax.y));

	// 自機の体力バーの大きさを変更
	oldLen = playerHpBar->getSize().x;
	nowLen = (float)player->getHp() / (float)playerHpMax * playerHpBarWidMax;
	playerHpBar->setSize(XMFLOAT2(std::lerp(oldLen, nowLen, 0.5f), playerHpBar->getSize().y));
}

void BossScene::update_killBoss()
{
	const auto nowTime = timer->getNowTime();
	constexpr auto endTime = Timer::oneSecF * 3;

	if (nowTime > endTime)
	{
		endKillBoss();
	} else
	{
		const float raito = (float)nowTime / (float)endTime;

		const float scale = std::lerp(killBossData->startBossScale,
									  killBossData->endBossScale,
									  raito * raito * raito * raito);

		boss->setScale(scale);
	}

	// パーティクルを毎フレーム出す
	createParticle(boss->calcWorldPos(), 32U, 16.f, 16.f);
}

void BossScene::update_end()
{
	SceneManager::getInstange()->changeScene<EndScene>();
}

void BossScene::startAppearBoss()
{
	// 照準は非表示
	aim2D->isInvisible = true;

	// カメラの情報を取っておく
	camParam =
		std::make_unique<CameraParam>(
			CameraParam{
				.parentObj = camera->getParentObj(),
				.angleRad = camera->getRelativeRotaDeg(),
				.eye2TargetLen = camera->getEye2TargetLen()
			}
	);

	// カメラをボス登場演出に合わせる
	camera->setParentObj(boss.get());
	constexpr float angle = 180.f;
	camera->setRelativeRotaDeg(XMFLOAT3(0.f, angle, 0.f));

	// 演出用情報
	appearBossData =
		std::make_unique<AppearBossData>(
			AppearBossData{
				.appearBossTime = static_cast<float>(Timer::oneSec * 5),
				.startCamLen = camParam->eye2TargetLen,
				.endCamLen = camParam->eye2TargetLen * 4.f,
				.startBossHpGrScale = 0.f,
				.endBossHpGrScale = hpGrSizeMax.x,
				.startCamAngle = angle,
				.endCamAngle = angle + 360.f,
				.startPlayerHpGrScale = 0.f,
				.endPlayerHpGrScale = playerHpBarWidMax
			}
	);

	update_proc = std::bind(&BossScene::update_appearBoss, this);
	timer->reset();
}

void BossScene::endAppearBoss()
{
	// 照準を表示
	aim2D->isInvisible = false;

	// ボス行動開始
	boss->setAlive(true);
	for (auto& i : attackableEnemy)
	{
		i->setAlive(true);
	}

	// カメラを自機に戻す
	camera->setParentObj(camParam->parentObj);
	camera->setRelativeRotaDeg(camParam->angleRad);
	camera->setEye2TargetLen(camParam->eye2TargetLen);

	bossHpGr->setSize(XMFLOAT2(appearBossData->endBossHpGrScale, bossHpGr->getSize().y));
	playerHpBar->setSize(XMFLOAT2(appearBossData->endPlayerHpGrScale, playerHpBar->getSize().y));

	// 関数を変える
	update_proc = std::bind(&BossScene::update_play, this);
}

void BossScene::startKillBoss()
{
	// 照準は消す
	aim2D->isInvisible = true;

	// ボスの体力は消す
	bossHpGr->isInvisible = true;

	// カメラの情報を取っておく
	camParam =
		std::make_unique<CameraParam>(
			CameraParam{
				.parentObj = camera->getParentObj(),
				.angleRad = camera->getRelativeRotaDeg(),
				.eye2TargetLen = camera->getEye2TargetLen()
			}
	);

	// カメラをボス登場演出に合わせる
	camera->setParentObj(boss.get());
	constexpr float angle = 180.f;
	camera->setRelativeRotaDeg(XMFLOAT3(0.f, angle, 0.f));

	// 演出用情報
	killBossData =
		std::make_unique<KillBossData>(
			KillBossData{ .startBossScale = boss->getScale(),
			.endBossScale = 0.f
			}
	);

	// ボス死亡演出開始
	update_proc = std::bind(&BossScene::update_killBoss, this);
	timer->reset();
}

void BossScene::endKillBoss()
{
	update_proc = std::bind(&BossScene::update_end, this);
}

void BossScene::additionalDrawObj3d()
{
	boss->drawWithUpdate(light.get());
	for (auto& i : bossParts)
	{
		i->drawWithUpdate(light.get());
	}
}

void BossScene::createParticle(const DirectX::XMFLOAT3& pos,
							   const uint16_t particleNum,
							   const float startScale,
							   const float vel,
							   const DirectX::XMFLOAT3& startCol,
							   const DirectX::XMFLOAT3& endCol)
{
	for (uint16_t i = 0U; i < particleNum; ++i)
	{
		const float theata = RandomNum::getRandf(0.f, XM_PI);
		const float phi = RandomNum::getRandf(0.f, XM_PI * 2.f);
		const float r = RandomNum::getRandf(0.f, vel);

		const XMFLOAT3 vel{
			r * dxBase->nearSin(theata) * dxBase->nearCos(phi),
			r * dxBase->nearCos(theata),
			r * dxBase->nearSin(theata) * dxBase->nearSin(phi)
		};

		constexpr float accNum = 10.f;
		const XMFLOAT3 acc = XMFLOAT3(vel.x / accNum,
									  vel.y / accNum,
									  vel.z / accNum);

		constexpr Timer::timeType life = Timer::oneSec / Timer::timeType(4);
		constexpr float endScale = 0.f;
		constexpr float startRota = 0.f, endRota = 0.f;

		// 追加
		particleMgr->add(life, pos, vel, acc,
						 startScale, endScale,
						 startRota, endRota,
						 startCol, endCol);
	}
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
		const float easeRate = -std::pow(2.f, 10.f * (1.f - raito) - 10.f) *
			DX12Base::ins()->nearSin((raito * 10.f - 10.75f) * c4);

		PostEffect::getInstance()->setRgbShiftNum({ easeRate * rgbShiftMumMax, 0.f });
	}
}

bool BossScene::addShotTarget(const std::forward_list<BaseEnemy*>& enemy,
							  const DirectX::XMFLOAT2& aim2DPosMin,
							  const DirectX::XMFLOAT2& aim2DPosMax)
{
	// 遠い敵を調べるためのもの
	float nowEnemyDistance{};
	BaseEnemy* farthestEnemyPt = nullptr;
	float farthestEnemyLen = 1.f;

	// 照準の中の敵の方へ弾を飛ばす
	for (BaseEnemy* i : enemy)
	{
		// いない敵は無視
		if (!i->getAlive()) { continue; }

		// 敵のスクリーン座標を取得
		const XMFLOAT2 screenEnemyPos = i->getObj()->calcScreenPos();

		// 敵が2D照準の中にいるかどうか
		if (aim2DPosMin.x <= screenEnemyPos.x &&
			aim2DPosMin.y <= screenEnemyPos.y &&
			aim2DPosMax.x >= screenEnemyPos.x &&
			aim2DPosMax.y >= screenEnemyPos.y)
		{
			// 敵との距離を更新
			nowEnemyDistance = std::sqrt(
				std::pow(i->getPos().x - camera->getEye().x, 2.f) +
				std::pow(i->getPos().y - camera->getEye().y, 2.f) +
				std::pow(i->getPos().z - camera->getEye().z, 2.f)
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
	player->setShotTarget(farthestEnemyPt);

	if (farthestEnemyPt)
	{
		aim2D->color = XMFLOAT4(1, 0, 0, 1);
		return true;
	}

	aim2D->color = XMFLOAT4(1, 1, 1, 1);
	return false;
}

void BossScene::movePlayer()
{
	// 上向きか否かの切り替え
	if (input->triggerKey(DIK_E) ||
		input->triggerPadButton(Input::PAD::X) ||
		input->triggerPadButton(Input::PAD::Y))
	{
		XMFLOAT3 camRrota = camera->getRelativeRotaDeg();

		constexpr float angle = 20.f;
		camRrota.x += playerUpTurn ? angle : -angle;

		playerUpTurn = !playerUpTurn;

		camera->setRelativeRotaDeg(camRrota);
	}

	// パッドの入力値
	XMFLOAT2 inputVal = input->getPadLStickRaito();

#pragma region 四方向入力キーボードとパッド十字ボタン

	// 移動
	{
		const bool hitW = input->hitKey(DIK_W) || input->hitKey(DIK_UP) || input->getPadButton(Input::PAD::UP);
		const bool hitA = input->hitKey(DIK_A) || input->hitKey(DIK_LEFT) || input->getPadButton(Input::PAD::LEFT);
		const bool hitS = input->hitKey(DIK_S) || input->hitKey(DIK_DOWN) || input->getPadButton(Input::PAD::DOWN);
		const bool hitD = input->hitKey(DIK_D) || input->hitKey(DIK_RIGHT) || input->getPadButton(Input::PAD::RIGHT);

		if (hitW)
		{
			inputVal.y = 1.f;
		} else if (hitS)
		{
			inputVal.y = -1.f;
		}

		// 回転
		if (hitA)
		{
			inputVal.x = -1.f;
		} else if (hitD)
		{
			inputVal.x = 1.f;
		}
	}

#pragma endregion 四方向入力キーボードとパッド十字ボタン

	// ゆっくりかどうか
	const bool slowFlag =
		input->hitKey(DIK_LCONTROL) ||
		input->getPadButton(Input::PAD::LEFT_THUMB);

	// ダッシュするかどうか
	const bool dashFlag =
		input->hitKey(DIK_LSHIFT) ||
		input->getPadButton(Input::PAD::LB);

	// 移動回転を反映
	if (inputVal.y != 0.f)
	{
		// 移動
		float moveSpeed = inputVal.y * 90.f / DX12Base::ins()->getFPS();

		if (slowFlag)
		{
			moveSpeed /= 2.f;
		} else if (dashFlag)
		{
			moveSpeed *= 2.f;
		}

		// 移動させる
		player->moveForward(moveSpeed);
	}
	if (inputVal.x != 0.f)
	{
		// 回転

		// 左右の回転
		float speed = inputVal.x * 45.f / DX12Base::ins()->getFPS();

		// 左シフトと左コントロールで速度変更
		if (input->hitKey(DIK_LCONTROL))
		{
			speed /= 2.f;
		} else if (dashFlag)
		{
			speed *= 2.f;
		}

		XMFLOAT3 rota = player->getRotation();

		// 回転させる
		rota.y += speed;

		player->setRotation(rota);
	}
}

void BossScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());
	bossHpGr->drawWithUpdate(DX12Base::ins(), spBase.get());
	aim2D->drawWithUpdate(DX12Base::ins(), spBase.get());

	playerHpBarEdge->drawWithUpdate(dxBase, spBase.get());
	playerHpBar->drawWithUpdate(dxBase, spBase.get());

	ImGui::SetNextWindowPos(ImVec2(fstWinPos.x,
								   fstWinPos.y));
	ImGui::SetNextWindowSize(ImVec2(300.f, 100.f));

	ImGui::Begin("ボス戦", nullptr, winFlags);
	ImGui::PushFont(dxBase->getBigImFont());
	ImGui::Text("イボを撃ち潰せ！");
	ImGui::PopFont();
	ImGui::Text("ボスの弾は迎撃せよ！");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth(), 150));
	ImGui::End();

	ImGui::Begin("操作", nullptr, winFlags);
	ImGui::Text("WS : 移動");
	ImGui::Text("AD : 回転");
	ImGui::Text("左シフト + 移動 or 回転 : ダッシュ");
	ImGui::Text("E : カメラ位置変更");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth(), 100));
	ImGui::End();

	ImGui::Begin("情報", nullptr, winFlags);
	ImGui::Text("自機体力 : %.2f%%", (float)player->getHp() / (float)playerHpMax * 100.f);
	if (boss->getAlive())
	{
		ImGui::Text("ボスHP : %.2f%% (%u)",
					(float)calcBossHp() / (float)bossHpMax * 100.f,
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