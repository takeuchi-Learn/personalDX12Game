﻿#include "BossScene.h"
#include "EndScene.h"
#include "GameOverScene.h"

#include <DirectXMath.h>
#include <imgui.h>
#include "System/SceneManager.h"
#include "System/PostEffect.h"
#include <Util/RandomNum.h>
#include <Util/Util.h>

#include "Collision/Collision.h"

#include <fstream>
#include <sstream>

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
	dxBase(DX12Base::ins()),
	input(Input::getInstance()),
	timer(std::make_unique<Timer>()),
	camera(std::make_unique<CameraObj>(nullptr)),
	light(std::make_unique<Light>()),
	update_proc(std::bind(&BossScene::update_start, this)),
	particleMgr(std::make_unique<ParticleMgr>(L"Resources/effect1.png", camera.get())),
	playerHpBarWidMax(WinAPI::window_width * 0.25f)
{
#pragma region 音

	killSe = std::make_unique<Sound>("Resources/SE/Sys_Set03-click.wav");
	bossDamageSe = std::make_unique<Sound>("Resources/SE/SNES-Shooter02-05(Bomb).wav");
	bgm = std::make_unique<Sound>("Resources/BGM/Sympathetic-Nerves.wav");

#pragma endregion 音

	// カメラ
	camera->setFarZ(10000.f);
	sceneChangeStartCamLen = camera->getEye2TargetLen() * 10.f;
	sceneChangeEndCamLen = camera->getEye2TargetLen();

	initGameObj();

	// カメラの親を自機にする
	camera->setParentObj(playerParent.get());

	initBackObj();

	initSprite();
}

void BossScene::initSprite()
{
	spBase = std::make_unique<SpriteBase>();

	aim2D = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/aimPos.png"),
									 spBase.get());

	const UINT hpBarTex = spBase->loadTexture(L"Resources/hpBar.png");

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

	// 体力バー
	hpBarModel = std::make_unique<ObjModel>("Resources/hpBar/", "hpBar");
	const auto& bossBar = hpBar.emplace("boss", std::make_unique<Object3d>(camera.get(), hpBarModel.get())).first->second;
	bossBar->scale.y = 5.f;
}

void BossScene::initPlayer()
{
	playerModel = std::make_unique<ObjModel>("Resources/player", "player");
	playerBulModel = std::make_unique<ObjModel>("Resources/bullet", "bullet", 0U, true);
	playerHpMax = 20U;

	playerParent = std::make_unique<GameObj>(camera.get());
	player = std::make_unique<Player>(camera.get(), playerModel.get(), XMFLOAT3(0.f, 0.f, 0.f));

	player->setParent(playerParent.get());
	// 大きさを設定
	player->setScale(10.f);

	playerHpMax = 20u;
	player->setScale(10.f);
	player->setHp(playerHpMax);

	player->setBulLife(600ui16);

	sceneChangeStartPos = XMFLOAT3(0.f, 500.f, 0.f);
	sceneChangeEndPos = XMFLOAT3(0.f, 0.f, 0.f);

	sceneChangeStartRota = playerParent->getRotation();
	sceneChangeStartRota.y += 90.f;
	sceneChangeEndRota = playerParent->getRotation();
}

void BossScene::initEnemy()
{
	// ボスの初期化
	initBoss();
}

void BossScene::initBoss()
{
	constexpr float bossScale = 100.f;

	boss->setScale(bossScale);
	boss->setPos(XMFLOAT3(0, boss->getScaleF3().y, 300));
	boss->setRotation(XMFLOAT3(0, 180.f, 0));
	boss->setTargetObj(player.get());
	boss->setSmallEnemyModel(bossModel.get());
	boss->getObj()->color = XMFLOAT4(2, 0.5f, 0.25f, 1);
	boss->setAlive(false);

	// ボスのパーツ
	bossPartsModel = std::make_unique<ObjModel>("Resources/koshi", "koshi", 0U, false);

	// パーツの位置をファイルから読み込む
	std::vector<DirectX::XMFLOAT3> bossPartsData;
	const auto bpdCsv = Util::loadCsv("Resources/bossPartsData.csv");
	for (auto& y : bpdCsv)
	{
		// 空行を飛ばす
		if (y.empty()) { continue; }

		// 列数
		const auto size = y.size();

		XMFLOAT3 pos{};
		if (size >= 1u) { pos.x = std::stof(y[0]); }
		if (size >= 2u) { pos.y = std::stof(y[1]); }
		if (size >= 3u) { pos.z = std::stof(y[2]); }

		bossPartsData.emplace_back(pos);
	}

	// 全要素の設定
	bossParts.reserve(bossPartsData.size());
	for (auto& bpd : bossPartsData)
	{
		auto& i = bossParts.emplace_back(std::make_shared<BaseEnemy>(camera.get(), bossPartsModel.get()));

		// 位置を設定
		i->setPos(bpd);

		// ボス本体を親とする
		i->setParent(boss->getObj());

		// 大きさを変更
		constexpr float bossPartsScale = 10.f / bossScale;
		i->setScale(bossPartsScale);

		// 体力を設定
		constexpr uint16_t hp = 10ui16;
		i->setHp(hp);
		bossHpMax += hp;

		// 攻撃可能な敵リストに追加
		attackableEnemy.emplace_front(i);
	}
}

void BossScene::initBackObj()
{
	backPipelineSet = Object3d::createGraphicsPipeline(Object3d::BLEND_MODE::ALPHA,
													   L"Resources/Shaders/BackVS.hlsl",
													   L"Resources/Shaders/BackPS.hlsl");

	// 背景の天球
	backModel.reset(new ObjModel("Resources/back/", "back", 0U, true));
	backObj.reset(new Object3d(camera.get(), backModel.get()));
	const float backScale = camera->getFarZ() * 0.9f;
	backObj->scale = XMFLOAT3(backScale, backScale, backScale);

	// 地面
	groundModel.reset(new ObjModel("Resources/ground", "ground"));
	groundObj.reset(new Object3d(camera.get(), groundModel.get()));
	constexpr UINT groundSize = 5000u;
	groundObj->position = XMFLOAT3(0, -player->getScale() * 500.f, 0);
	groundObj->scale = XMFLOAT3(groundSize, groundSize, groundSize);

	constexpr float tillingNum = (float)groundSize / 32.f;
	groundModel->setTexTilling(XMFLOAT2(tillingNum, tillingNum));
}

uint32_t BossScene::calcBossHp() const
{
	uint32_t bossHp = 0ui32;
	for (auto& i : bossParts)
	{
		if (i->getAlive())
		{
			bossHp += (uint32_t)i->getHp();
		}
	}
	return bossHp;
}

void BossScene::start()
{
	PostEffect::getInstance()->setVignIntensity(0.5f);
	PostEffect::getInstance()->setSpeedLineIntensity(0.125f);

	// 照準の位置を画面中央にする
	input->setMousePos(WinAPI::window_width / 2, WinAPI::window_height / 2);
	player->setAim2DPos(XMFLOAT2((float)WinAPI::window_width / 2.f, (float)WinAPI::window_height / 2.f));
	aim2D->position.x = player->getAim2DPos().x;
	aim2D->position.y = player->getAim2DPos().y;

	aim2D->isInvisible = true;

	for (auto& i : attackableEnemy)
	{
		i.lock()->setAlive(false);
	}

	// bgm鳴らす
	Sound::SoundPlayWave(bgm.get(), XAUDIO2_LOOP_INFINITE, 0.2f);

	timer->reset();
}

void BossScene::update()
{
	rotaBackObj();

	moveAim2DPos();

	// 更新処理本体
	update_proc();
	camera->update();

	// 背景オブジェクトの中心をカメラにする
	backObj->position = camera->getEye();
	// ライトはカメラの位置にする
	light->setLightPos(camera->getEye());

	// ライトとカメラの更新
	light->update();
}

#pragma endregion 初期化

void BossScene::update_start()
{
#ifdef _DEBUG
	if (input->triggerKey(DIK_SPACE))
	{
		playerParent->setPos(sceneChangeEndPos);
		playerParent->setRotation(sceneChangeEndRota);
		camera->setEye2TargetLen(sceneChangeEndCamLen);

		startAppearBoss();
		return;
	}
#endif // _DEBUG

	if (timer->getNowTime() > sceneChangeTime)
	{
		playerParent->setPos(sceneChangeEndPos);
		playerParent->setRotation(sceneChangeEndRota);
		camera->setEye2TargetLen(sceneChangeEndCamLen);

		startAppearBoss();
		return;
	}

	const float raito = (float)timer->getNowTime() / (float)sceneChangeTime;

	playerParent->setPos(lerp(sceneChangeStartPos, sceneChangeEndPos, raito));
	playerParent->setRotation(lerp(sceneChangeStartRota, sceneChangeEndRota, raito));

	const float camLen = std::lerp(sceneChangeStartCamLen, sceneChangeEndCamLen, raito);
	camera->setEye2TargetLen(camLen);
}

void BossScene::update_appearBoss()
{
	// デバッグ時はスペースで演出終了
#ifdef _DEBUG

	if (input->triggerKey(DIK_SPACE))
	{
		endAppearBoss();
		return;
	}

#endif // _DEBUG

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
	hpBar.at("boss")->scale.x = scaleRaito;

	scaleRaito = std::lerp(appearBossData->startPlayerHpGrScale,
						   appearBossData->endPlayerHpGrScale,
						   barRaito);
	playerHpBar->setSize(XMFLOAT2(scaleRaito, playerHpBar->getSize().y));
}

void BossScene::update_play()
{
#ifdef _DEBUG

	if (Input::getInstance()->triggerKey(DIK_SPACE))
	{
		update_proc = std::bind(&BossScene::update_end<EndScene>, this);
	}

#endif // _DEBUG

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

		std::forward_list<std::weak_ptr<BaseEnemy>> inAim2DEnemy = attackableEnemy;
		for (auto& i : boss->getSmallEnemyList())
		{
			inAim2DEnemy.emplace_front(i);
		}
		addShotTarget(inAim2DEnemy, aim2DMin, aim2DMax);

		// --------------------
		// 弾発射
		// --------------------
		if (!player->getShotTarget().expired())
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
			if (e.expired()) { continue; }
			auto i = e.lock();

			// いない敵は判定しない
			if (!i->getAlive()) { continue; }

			const CollisionShape::Sphere enemy(XMLoadFloat3(&i->calcWorldPos()),
											   i->getScale());

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
					if (i->damage(1u, true))
					{
						i->setDrawFlag(false);
						// 赤エフェクトを出す
						createParticle(i->calcWorldPos(), 128U, 16.f, 16.f, XMFLOAT3(1.f, 0.25f, 0.25f));
						Sound::SoundPlayWave(killSe.get(), 0, 0.2f);
					} else
					{
						// シアンエフェクトを出す
						createParticle(i->calcWorldPos(), 96U, 12.f, 12.f, XMFLOAT3(0.25f, 1.f, 1.f));
						Sound::SoundPlayWave(bossDamageSe.get(), 0, 0.2f);
					}
				}
			}
		}

		// --------------------
		// 自機とボス弾(雑魚敵)の当たり判定
		// --------------------
		if (player->getAlive())
		{
			const CollisionShape::Sphere pCol(XMLoadFloat3(&player->calcWorldPos()),
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
						update_proc = std::bind(&BossScene::update_end<GameOverScene>, this);
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

					Sound::SoundPlayWave(killSe.get(), 0, 0.2f);
				}
			}
		}

		if (boss->getAlive())
		{
			bool alive = false;
			for (auto& i : bossParts)
			{
				// bossPartsが全て死んでいたらボスは死ぬ
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

	// 自機の体力バーの大きさを変更
	float oldLen = playerHpBar->getSize().x;
	float nowLen = (float)player->getHp() / (float)playerHpMax * playerHpBarWidMax;
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

template<class NextScene>
void BossScene::update_end()
{
	// BGM止める
	Sound::SoundStopWave(bgm.get());

	// 次のシーンへ進む
	SceneManager::getInstange()->changeScene<NextScene>();
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
				.endBossHpGrScale = boss->getScaleF3().x,
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
		i.lock()->setAlive(true);
	}

	// カメラを自機に戻す
	camera->setParentObj(camParam->parentObj);
	camera->setRelativeRotaDeg(camParam->angleRad);
	camera->setEye2TargetLen(camParam->eye2TargetLen);

	hpBar.at("boss")->scale.x = appearBossData->endBossHpGrScale;
	playerHpBar->setSize(XMFLOAT2(appearBossData->endPlayerHpGrScale, playerHpBar->getSize().y));

	// 関数を変える
	update_proc = std::bind(&BossScene::update_play, this);
}

void BossScene::startKillBoss()
{
	// 照準は消す
	aim2D->isInvisible = true;

	// ボスの体力は消す
	hpBar.at("boss")->drawFlag = false;

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
	update_proc = std::bind(&BossScene::update_end<EndScene>, this);
}

void BossScene::drawObj3d()
{
	backObj->drawWithUpdate(light.get(), backPipelineSet);

	groundObj->drawWithUpdate(light.get());

	playerParent->update();
	player->drawWithUpdate(light.get());

	for (auto& i : attackableEnemy)
	{
		i.lock()->drawWithUpdate(light.get());
	}

	boss->drawWithUpdate(light.get());
	for (auto& i : bossParts)
	{
		i->drawWithUpdate(light.get());
	}

	updateBossHpBar();
	for (auto& i : hpBar)
	{
		i.second->drawWithUpdate(light.get());
	}

	particleMgr->drawWithUpdate();
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

bool BossScene::addShotTarget(const std::forward_list<std::weak_ptr<BaseEnemy>>& enemy,
							  const DirectX::XMFLOAT2& aim2DPosMin,
							  const DirectX::XMFLOAT2& aim2DPosMax)
{
	// 遠い敵を調べるためのもの
	float nowEnemyDistance{};
	std::weak_ptr<BaseEnemy> farthestEnemyPt;
	float farthestEnemyLen = 1.f;

	// 照準の中の敵の方へ弾を飛ばす
	for (auto& e : enemy)
	{
		auto i = e.lock();
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
	if (farthestEnemyPt.expired())
	{
		player->deleteShotTarget();
		aim2D->color = XMFLOAT4(1, 1, 1, 1);
		return false;
	}

	player->setShotTarget(farthestEnemyPt);
	aim2D->color = XMFLOAT4(1, 0, 0, 1);
	return true;
}

void BossScene::movePlayer()
{
	// 上向きか否かの切り替え
	if (input->triggerKey(DIK_E) ||
		input->triggerPadButton(Input::PAD::X) ||
		input->triggerPadButton(Input::PAD::Y))
	{
		XMFLOAT3 camRota = camera->getRelativeRotaDeg();

		constexpr float angle = 20.f;
		camRota.x += playerUpTurn ? angle : -angle;

		playerUpTurn = !playerUpTurn;

		camera->setRelativeRotaDeg(camRota);
	}

	// パッドの入力値
	XMFLOAT2 inputVal = input->getPadLStickRaito();

	// 無効な入力は0にする
	if (!input->isVaildPadLStickX())
	{
		inputVal.x = 0.f;
	}
	if (!input->isVaildPadLStickY())
	{
		inputVal.y = 0.f;
	}

#pragma region 四方向入力キーボードとパッド十字ボタン

	// 上下
	if (inputVal.y == 0.f)
	{
		if (input->hitKey(DIK_W) || input->hitKey(DIK_UP) || input->getPadButton(Input::PAD::UP))
		{
			inputVal.y = 1.f;
		} else if (input->hitKey(DIK_S) || input->hitKey(DIK_DOWN) || input->getPadButton(Input::PAD::DOWN))
		{
			inputVal.y = -1.f;
		}
	}

	// 左右
	if (inputVal.x == 0.f)
	{
		if (input->hitKey(DIK_A) || input->hitKey(DIK_LEFT) || input->getPadButton(Input::PAD::LEFT))
		{
			inputVal.x = -1.f;
		} else if (input->hitKey(DIK_D) || input->hitKey(DIK_RIGHT) || input->getPadButton(Input::PAD::RIGHT))
		{
			inputVal.x = 1.f;
		}
	}

#pragma endregion 四方向入力キーボードとパッド十字ボタン

	const bool moveYFlag = inputVal.y != 0.f;
	const bool moveXFlag = inputVal.x != 0.f;

	float playerRot = 0.f;

	if (moveYFlag || moveXFlag)
	{
		// 入力値を0~1にする
		const float len = std::sqrt(
			inputVal.x * inputVal.x +
			inputVal.y * inputVal.y
		);
		if (len > 1.f)
		{
			inputVal.x /= len;
			inputVal.y /= len;
		}

		// 移動する速さ
		float speed = 90.f / DX12Base::ins()->getFPS();

		// 高速と低速
		if (input->hitKey(DIK_LCONTROL) ||
			input->getPadButton(Input::PAD::LEFT_THUMB))
		{
			speed /= 2.f;
		} else if (input->hitKey(DIK_LSHIFT) ||
				   input->getPadButton(Input::PAD::LB))
		{
			speed *= 2.f;
		}

		// 移動させる
		if (moveYFlag)
		{
			playerParent->moveForward(inputVal.y * speed);
		}
		if (moveXFlag)
		{
			playerParent->moveRight(inputVal.x * speed);
			playerRot = 45.f * -inputVal.x;
		}
	}
	player->setRotation(XMFLOAT3(player->getRotation().x,
								 player->getRotation().y,
								 playerRot));
}

void BossScene::moveAim2DPos()
{
	constexpr POINT center = POINT(WinAPI::window_width / 2,
								   WinAPI::window_height / 2);

	// マウスカーソルの位置をパッド入力に合わせてずらす
	const POINT pos = input->getMousePos();
	input->setMousePos(center.x, center.y);

	POINT posDiff = POINT(pos.x - center.x,
						  pos.y - center.y);

	// 右スティックの入力で速度を決める
	XMFLOAT2 rStick = input->getPadRStickRaito();
	float stickSpeed = 10.f;
	if (input->getPadButton(Input::PAD::RIGHT_THUMB))
	{
		stickSpeed /= 2.f;
	}

	// 入力に合わせて移動
	posDiff.x += static_cast<LONG>(rStick.x * stickSpeed);
	posDiff.y += static_cast<LONG>(-rStick.y * stickSpeed);

	// 自機回転速度
	constexpr float rotaSpeed = 0.125f;

	// 入力に合わせて回転
	XMFLOAT3 rota = playerParent->getRotation();
	rota.x += rotaSpeed * posDiff.y;
	rota.y += rotaSpeed * posDiff.x;
	playerParent->setRotation(rota);
}

void BossScene::rotaBackObj()
{
	// シーン遷移中も背景は回す
	XMFLOAT2 shiftUv = backModel->getShiftUv();
	constexpr float shiftSpeed = 0.01f;

	shiftUv.x += shiftSpeed / DX12Base::getInstance()->getFPS();

	backModel->setShivtUv(shiftUv);
}

void BossScene::updateBossHpBar()
{
	if (!boss->getDrawFlag()) { return; }

	const auto& bossBar = hpBar.at("boss");
	if (!bossBar->drawFlag) { return; }

	const XMFLOAT3 bossPos = boss->calcWorldPos();

	// カメラを向く
	XMFLOAT3 velF3 = camera->getEye();
	velF3.x -= bossPos.x;
	velF3.y -= bossPos.y;
	velF3.z -= bossPos.z;
	const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(velF3);
	bossBar->rotation.x = rotaDeg.x;
	bossBar->rotation.y = rotaDeg.y;

	/// その他更新
	bossBar->position = bossPos;
	bossBar->position.y += boss->getScaleF3().y + bossBar->scale.y;
	bossBar->scale.x = std::lerp(bossBar->scale.x,
							   (float)calcBossHp() / (float)bossHpMax * boss->getScaleF3().x,
							   0.5f);
}

void BossScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());
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
	ImGui::Text("ボスの弾は迎撃可能！");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth(), 150));
	ImGui::End();

	ImGui::Begin("操作", nullptr, winFlags);
	ImGui::Text("WASD : 移動");
	ImGui::Text("左Shift : ダッシュ");
	ImGui::Text("左Ctrl : ゆっくり");
	ImGui::Text("E : カメラ位置変更");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth(), 100));
	ImGui::End();

	/*ImGui::Begin("情報", nullptr, winFlags);
	ImGui::Text("自機体力 : %.2f%%(%u / %u)",
				(float)player->getHp() / (float)playerHpMax * 100.f,
				player->getHp(),
				playerHpMax);
	if (boss->getAlive())
	{
		const uint32_t bossHp = calcBossHp();
		ImGui::Text("ボスHP : %.2f%% (%u)",
					(float)bossHp / (float)bossHpMax * 100.f,
					bossHp);
	}
	ImGui::End();*/
}

BossScene::~BossScene()
{
	PostEffect::getInstance()->setRgbShiftNum({ 0.f, 0.f });

	PostEffect::getInstance()->setSpeedLineIntensity(0.f);
	PostEffect::getInstance()->setVignIntensity(0.25f);

	PostEffect::getInstance()->changePipeLine(0U);
}