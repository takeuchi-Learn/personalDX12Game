#include "BossScene.h"
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
	inline ImVec2 f2ToIV2(const XMFLOAT2& f2)
	{
		return ImVec2(f2.x, f2.y);
	}

	constexpr XMFLOAT3 killEffCol = XMFLOAT3(1.f, 0.25f, 0.25f);
	constexpr XMFLOAT3 noKillEffCol = XMFLOAT3(0.25f, 1.f, 1.f);
}

#pragma region 初期化

BossScene::BossScene() :
	dxBase(DX12Base::ins()),
	input(Input::getInstance()),
	timer(std::make_unique<Timer>()),
	camera(std::make_unique<CameraObj>(nullptr)),
	light(std::make_unique<Light>()),
	update_proc(std::bind(&BossScene::update_start, this)),
	particleMgr(std::make_unique<ParticleMgr>(L"Resources/effect1.png", camera.get()))
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

	aimGrNum = spBase->loadTexture(L"Resources/aimPos.png");
	cursorGr = std::make_unique<Sprite>(aimGrNum, spBase.get());
}

void BossScene::initGameObj()
{
	initPlayer();

	initEnemy();
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
	bossBulModel = std::make_unique<ObjModel>("Resources/sphere", "sphere");
	boss = std::make_unique<BossEnemy>(camera.get(), nullptr);

	boss->setPos(XMFLOAT3(0, boss->getScaleF3().y, 800));
	boss->setRotation(XMFLOAT3(0, 180.f, 0));
	boss->setTargetObj(player.get());
	boss->setBulModel(bossBulModel.get());
	boss->getObj()->color = XMFLOAT4(2, 0.5f, 0.25f, 1);
	boss->setAlive(false);

	// ボスのパーツ
	bossPartsModel = std::make_unique<ObjModel>("Resources/koshi", "koshi", 0U, false);

	// パーツの位置をファイルから読み込む
	{
		struct BossPartsData
		{
			XMFLOAT3 pos;
			float scale;
		};
		std::forward_list<BossPartsData> bossPartsData;
		const auto bpdCsv = Util::loadCsv("Resources/bossPartsData.csv");
		for (auto& y : bpdCsv)
		{
			// 空行を飛ばす
			if (y.empty()) { continue; }

			// 列数
			const auto size = y.size();

			XMFLOAT3 pos{};
			float scale = 1.f;
			if (size >= 1u)
			{
				pos.x = std::stof(y[0]);

				if (size >= 2u)
				{
					pos.y = std::stof(y[1]);

					if (size >= 3u)
					{
						pos.z = std::stof(y[2]);

						if (size >= 4u)
						{
							scale = std::stof(y[3]);
						}
					}
				}
			}

			bossPartsData.emplace_front(BossPartsData{ .pos = pos, .scale = scale });
		}

		// 全要素の設定
		for (auto& bpd : bossPartsData)
		{
			auto& i = bossParts.emplace_back(std::make_shared<BaseEnemy>(camera.get(), bossPartsModel.get()));

			// 位置を設定
			i->setPos(bpd.pos);

			// ボス本体を親とする
			i->setParent(boss->getObj());

			// 大きさを変更
			i->setScale(bpd.scale);

			// 体力を設定
			constexpr uint16_t hp = 10ui16;
			i->setHp(hp);
			bossHpMax += hp;

			// 攻撃可能な敵リストに追加
			attackableEnemy.emplace_front(i);
		}
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
	cursorGr->position.x = player->getAim2DPos().x;
	cursorGr->position.y = player->getAim2DPos().y;

	cursorGr->isInvisible = true;

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

	// RGBずらしの更新
	updateRgbShift();

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

	playerParent->setPos(Util::lerp(sceneChangeStartPos, sceneChangeEndPos, raito));
	playerParent->setRotation(Util::lerp(sceneChangeStartRota, sceneChangeEndRota, raito));

	const float camLen = std::lerp(sceneChangeStartCamLen, sceneChangeEndCamLen, raito);
	camera->setEye2TargetLen(camLen);

	// 体力バー
	float barRaito = 1.f - raito;
	barRaito *= barRaito * barRaito * barRaito;
	playerHpBar.backNowRaito = 1.f - barRaito;
	playerHpBar.frontNowRaito = playerHpBar.backNowRaito;

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

	bossHpBar.frontNowRaito = std::lerp(appearBossData->startBossHpGrScale,
										appearBossData->endBossHpGrScale,
										barRaito);
	bossHpBar.backNowRaito = bossHpBar.frontNowRaito;
}

void BossScene::update_play()
{
#ifdef _DEBUG

	if (Input::getInstance()->hitKey(DIK_LSHIFT) &&
		Input::getInstance()->triggerKey(DIK_SPACE))
	{
		update_proc = std::bind(&BossScene::update_end<EndScene>, this);
	}

#endif // _DEBUG

	if (player->getAlive())
	{
		// --------------------
		// 自機の移動(と回転)
		// --------------------
		movePlayer();

		// --------------------
		// 弾発射
		// --------------------
		if (input->hitMouseButton(Input::MOUSE::LEFT) ||
			input->hitPadButton(Input::PAD::RB) ||
			input->hitPadButton(Input::PAD::A) ||
			input->hitPadButton(Input::PAD::B))
		{
			cursorGr->color = XMFLOAT4(1, 0, 0, 1);

			const XMFLOAT2 aim2DMin = XMFLOAT2(input->getMousePos().x - cursorGr->getSize().x / 2.f,
											  input->getMousePos().y - cursorGr->getSize().y / 2.f);
			const XMFLOAT2 aim2DMax = XMFLOAT2(input->getMousePos().x + cursorGr->getSize().x / 2.f,
											   input->getMousePos().y + cursorGr->getSize().y / 2.f);

			addShotTarget(attackableEnemy, aim2DMin, aim2DMax);

		} else if (input->releaseTriggerMouseButton(Input::MOUSE::LEFT) ||
				   input->releaseTriggerMouseButton(Input::PAD::RB) ||
				   input->releaseTriggerMouseButton(Input::PAD::A) ||
				   input->releaseTriggerMouseButton(Input::PAD::B))
		{
			if (player->shotAll(camera.get(), playerBulModel.get(), 2.f))
			{
				reticle.clear();

				cursorGr->color = XMFLOAT4(1, 1, 1, 1);
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
						ParticleMgr::createParticle(particleMgr.get(), i->calcWorldPos(), 128U, 16.f, 16.f, killEffCol);
						Sound::SoundPlayWave(killSe.get(), 0, 0.2f);
					} else
					{
						// シアンエフェクトを出す
						ParticleMgr::createParticle(particleMgr.get(), i->calcWorldPos(), 96U, 12.f, 12.f, noKillEffCol);
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

			for (auto& i : boss->getBulList())
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
	float oldLen = playerHpBar.backNowRaito;
	playerHpBar.frontNowRaito = (float)player->getHp() / (float)playerHpMax;
	playerHpBar.backNowRaito = std::lerp(oldLen, playerHpBar.frontNowRaito, 0.125f);

	oldLen = bossHpBar.backNowRaito;
	bossHpBar.frontNowRaito = (float)calcBossHp() / (float)bossHpMax;
	bossHpBar.backNowRaito = std::lerp(oldLen, bossHpBar.frontNowRaito, 0.125f);
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
	ParticleMgr::createParticle(particleMgr.get(), boss->calcWorldPos(), 32U, 16.f, 16.f);
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
	cursorGr->isInvisible = true;

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
				.appearBossTime = static_cast<float>(Timer::oneSec) * 5.f,
				.startCamLen = camParam->eye2TargetLen,
				.endCamLen = camParam->eye2TargetLen * 4.f,
				.startBossHpGrScale = 0.f,
				.endBossHpGrScale = 1.f,
				.startCamAngle = angle,
				.endCamAngle = angle + 360.f,
			}
	);

	update_proc = std::bind(&BossScene::update_appearBoss, this);
	timer->reset();
}

void BossScene::endAppearBoss()
{
	// 照準を表示
	cursorGr->isInvisible = false;

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

	bossHpBar.frontNowRaito = appearBossData->endBossHpGrScale;
	bossHpBar.backNowRaito = bossHpBar.frontNowRaito;

	// 関数を変える
	update_proc = std::bind(&BossScene::update_play, this);
}

void BossScene::startKillBoss()
{
	// 照準は消す
	cursorGr->isInvisible = true;

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

	particleMgr->drawWithUpdate();
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
	// 撃ったかどうか（戻り値用）
	bool noShot = true;

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

			if (player->addShotTarget(i))
			{
				auto& ref = reticle.emplace_front(aimGrNum, spBase.get());

				ref.target = i;

				ref.sprite->color = XMFLOAT4(1, 0, 0, 1);

				const auto size = ref.sprite->getSize();
				ref.sprite->setSize(XMFLOAT2(size.x / 2.f, size.y / 2.f));

				noShot = false;
			}
		}
	}

	return noShot;
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
	XMFLOAT2 inputVal = input->hitPadLStickRaito();

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
		if (input->hitKey(DIK_W) || input->hitKey(DIK_UP) || input->hitPadButton(Input::PAD::UP))
		{
			inputVal.y = 1.f;
		} else if (input->hitKey(DIK_S) || input->hitKey(DIK_DOWN) || input->hitPadButton(Input::PAD::DOWN))
		{
			inputVal.y = -1.f;
		}
	}

	// 左右
	if (inputVal.x == 0.f)
	{
		if (input->hitKey(DIK_A) || input->hitKey(DIK_LEFT) || input->hitPadButton(Input::PAD::LEFT))
		{
			inputVal.x = -1.f;
		} else if (input->hitKey(DIK_D) || input->hitKey(DIK_RIGHT) || input->hitPadButton(Input::PAD::RIGHT))
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
		if (float len = std::sqrt(inputVal.x * inputVal.x +
								  inputVal.y * inputVal.y) > 1.f)
		{
			inputVal.x /= len;
			inputVal.y /= len;
		}

		// 移動する速さ
		float speed = 90.f / DX12Base::ins()->getFPS();

		// 高速と低速
		if (input->hitKey(DIK_LCONTROL) ||
			input->hitPadButton(Input::PAD::LEFT_THUMB))
		{
			speed /= 2.f;
		} else if (input->hitKey(DIK_LSHIFT) ||
				   input->hitPadButton(Input::PAD::LB))
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
	const POINT prePos = input->getMousePos();
	input->setMousePos(center.x, center.y);

	POINT posDiff = POINT(prePos.x - center.x,
						  prePos.y - center.y);

	// 右スティックの入力で速度を決める
	XMFLOAT2 rStick = input->hitPadRStickRaito();
	float stickSpeed = 10.f;
	if (input->hitPadButton(Input::PAD::RIGHT_THUMB))
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

void BossScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());

	for (auto& i : reticle)
	{
		i.drawWithUpdate(spBase.get());
	}

	cursorGr->drawWithUpdate(DX12Base::ins(), spBase.get());

	// 自機の体力バー
	if (0.f < playerHpBar.backNowRaito)
	{
		// 自機体力
		constexpr XMFLOAT2 hpWinSize = XMFLOAT2(playerHpBarWidMax, (float)WinAPI::window_height / 32.f);
		constexpr XMFLOAT2 hpWinPosLT = XMFLOAT2(WinAPI::window_width / 20.f,
												 WinAPI::window_height - hpWinSize.y * 2.f);

		// 縁の大きさ
		constexpr XMFLOAT2 shiftVal = XMFLOAT2(hpWinSize.x / 40.f, hpWinSize.y / 4.f);

		// 余白を少し残す
		constexpr auto posLT = XMFLOAT2(hpWinPosLT.x + shiftVal.x,
										hpWinPosLT.y + shiftVal.y);

		// ウインドウの位置と大きさを指定
		ImGui::SetNextWindowPos(f2ToIV2(hpWinPosLT));
		ImGui::SetNextWindowSize(f2ToIV2(hpWinSize));

		ImGui::Begin("自機体力", nullptr, winFlags);
		const ImVec2 size = ImGui::GetWindowSize();

		// ウインドウ内のバーの大きさ
		const ImVec2 barSize = ImVec2(size.x - shiftVal.x * 2.f,
									  size.y - shiftVal.y * 2.f);

		ImVec2 posRB = ImVec2(posLT.x + barSize.x * playerHpBar.backNowRaito,
							  posLT.y + barSize.y);

		ImGui::GetWindowDrawList()->AddRectFilled(
			f2ToIV2(posLT), posRB,
			ImU32(0xff2222f8)
		);

		posRB.x = posLT.x + barSize.x * playerHpBar.frontNowRaito;

		ImGui::GetWindowDrawList()->AddRectFilled(
			f2ToIV2(posLT), posRB,
			ImU32(0xfff8f822)
		);

		ImGui::End();
	}

	// ボス体力バー
	if (0.f < bossHpBar.frontNowRaito)
	{
		// 自機体力
		constexpr XMFLOAT2 hpWinSize = XMFLOAT2(bossHpBarWidMax, (float)WinAPI::window_height / 32.f);
		constexpr XMFLOAT2 hpWinPosCT = XMFLOAT2(WinAPI::window_width / 2.f, hpWinSize.y * 2.f);

		// 縁の大きさ
		constexpr XMFLOAT2 shiftVal = XMFLOAT2(hpWinSize.x / 40.f, hpWinSize.y / 4.f);

		// ウインドウの位置と大きさを指定
		ImGui::SetNextWindowPos(f2ToIV2(hpWinPosCT), 0, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(f2ToIV2(hpWinSize));

		ImGui::Begin("ボス体力", nullptr, winFlags);
		const ImVec2 size = ImGui::GetWindowSize();

		// ウインドウ内のバーの大きさ
		const ImVec2 barSizeMax = ImVec2(size.x - shiftVal.x * 2.f,
										 size.y - shiftVal.y * 2.f);

		float barHalfWid = barSizeMax.x * bossHpBar.backNowRaito / 2.f;

		ImVec2 posLT = ImVec2(hpWinPosCT.x - barHalfWid,
							  hpWinPosCT.y - barSizeMax.y / 2.f);

		ImVec2 posRB = ImVec2(hpWinPosCT.x + barHalfWid,
							  hpWinPosCT.y + barSizeMax.y / 2.f);

		ImGui::GetWindowDrawList()->AddRectFilled(
			posLT, posRB,
			ImU32(0xff2222f8)
		);

		barHalfWid = barSizeMax.x * bossHpBar.frontNowRaito / 2.f;

		posLT.x = hpWinPosCT.x - barHalfWid;
		posRB.x = hpWinPosCT.x + barHalfWid;

		ImGui::GetWindowDrawList()->AddRectFilled(
			posLT, posRB,
			ImU32(0xfff8f822)
		);

		ImGui::End();
	}
}

BossScene::~BossScene()
{
	PostEffect::getInstance()->setRgbShiftNum({ 0.f, 0.f });

	PostEffect::getInstance()->setSpeedLineIntensity(0.f);
	PostEffect::getInstance()->setVignIntensity(0.25f);

	PostEffect::getInstance()->changePipeLine(0U);
}