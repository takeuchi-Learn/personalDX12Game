#include "RailShoot.h"
#include <DirectXMath.h>

#include "SceneManager.h"
#include "PlayScene.h"

#include "PostEffect.h"

#include "Collision.h"

#include "RandomNum.h"

using namespace DirectX;

const Time::timeType RailShoot::sceneChangeTime = Time::oneSec;

RailShoot::RailShoot()
	: dxBase(DX12Base::getInstance()),
	input(Input::getInstance()),

	update_proc(std::bind(&RailShoot::update_start, this)),

	camera(std::make_unique<Camera>(WinAPI::WinAPI::getInstance()->getWindowSize())),
	light(std::make_unique<Light>()),

	timer(std::make_unique<Time>()),

	spriteBase(std::make_unique<SpriteBase>()),

	back(std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true)),
	enemyModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),
	playerModel(std::make_unique<ObjModel>("Resources/box", "box")),

	playerBulModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),

	particleMgr(std::make_unique<ParticleMgr>(L"Resources/effect1.png", camera.get())),

	startSceneChangeTime(0U),

	backPipelineSet(Object3d::createGraphicsPipeline(Object3d::BLEND_MODE::ALPHA,
													 L"Resources/Shaders/BackVS.hlsl",
													 L"Resources/Shaders/BackPS.hlsl")) {
	// スプライト初期化
	const UINT debugTextTexNumber = spriteBase->loadTexture(L"Resources/debugfont.png");
	debugText = std::make_unique<DebugText>(debugTextTexNumber, spriteBase.get());

	// カメラ初期化
	camera->setFarZ(5000.f);
	camera->setEye(XMFLOAT3(0, WinAPI::getInstance()->getWindowSize().y * 0.06f, -180));	// 視点座標
	camera->setTarget(XMFLOAT3(0, 0, 0));	// 注視点座標
	camera->setUp(XMFLOAT3(0, 1, 0));		// 上方向
	camera->update();

	// ライト初期化
	light->setLightPos(camera->getEye());


	// 自機初期化
	constexpr XMFLOAT3 playerStartPos = XMFLOAT3(0, 0, 0);
	player = std::make_unique<Player>(camera.get(), playerModel.get(), playerStartPos);
	player->setScale(10.f);

	// 敵初期化
	constexpr size_t enemyNumDef = 1U;
	constexpr XMFLOAT3 enemyPosDef = XMFLOAT3(playerStartPos.x,
											  playerStartPos.y - 20.f,
											  playerStartPos.z + 300.f);
	enemy.resize(enemyNumDef);
	for (auto &i : enemy) {
		i = std::make_unique<Enemy>(camera.get(), enemyModel.get(), enemyPosDef);
		i->setVel(XMFLOAT3(0, 0, -1.f));
		i->setPos(XMFLOAT3(0, 0, 100));
		i->setScale(5.f);
	}

	// 天球
	const float backScale = camera->getFarZ() * 0.9f;
	back->setScale({ backScale, backScale, backScale });

	WinAPI::getInstance()->setWindowSize(WinAPI::window_width, WinAPI::window_height);
}

void RailShoot::start() {
	// タイマー開始
	timer->reset();
	startSceneChangeTime = timer->getNowTime();
}

void RailShoot::update() {
	{
		// シーン遷移中も背景は回す
		XMFLOAT3 backRota = back->getRotation();
		backRota.y += 6.f / dxBase->getFPS();
		back->setRotation(backRota);
	}

	// 背景オブジェクトの中心をカメラにする
	back->setPos(camera->getEye());

	// 主な処理
	update_proc();

	// ライトとカメラの更新
	light->update();
	camera->update();
}

void RailShoot::createParticle(const DirectX::XMFLOAT3 &pos,
							   const UINT particleNum,
							   const float startScale,
							   const float vel) {
	for (UINT i = 0U; i < particleNum; ++i) {

		const float theata = RandomNum::getRandf(0, XM_PI);
		const float phi = RandomNum::getRandf(0, XM_PI * 2.f);
		const float r = RandomNum::getRandf(0, vel);

		const XMFLOAT3 vel{
			r * dxBase->nearSin(theata) * dxBase->nearCos(phi),
			r * dxBase->nearCos(theata),
			r * dxBase->nearSin(theata) * dxBase->nearSin(phi)
		};

		constexpr float accNum = 10.f;
		const XMFLOAT3 acc = XMFLOAT3(vel.x / accNum,
									  vel.y / accNum,
									  vel.z / accNum);

		constexpr XMFLOAT3 startCol = XMFLOAT3(1, 1, 0.25f), endCol = XMFLOAT3(1, 0, 1);
		constexpr int life = Time::oneSec / 4;
		constexpr float endScale = 0.f;
		constexpr float startRota = 0.f, endRota = 0.f;

		// 追加
		particleMgr->add(std::make_unique<Time>(),
						 life, pos, vel, acc,
						 startScale, endScale,
						 startRota, endRota,
						 startCol, endCol);
	}
}

void RailShoot::changeNextScene() {
	PostEffect::getInstance()->changePipeLine(0U);

	update_proc = std::bind(&RailShoot::update_end, this);
	startSceneChangeTime = timer->getNowTime();
}

void RailShoot::update_start() {
	const Time::timeType nowTime = timer->getNowTime() - startSceneChangeTime;
	if (nowTime >= sceneChangeTime) {
		PostEffect::getInstance()->changePipeLine(SceneManager::getInstange()->getPostEff2Num());

		update_proc = std::bind(&RailShoot::update_play, this);
	}

	const float timeRaito = (float)nowTime / sceneChangeTime;
	PostEffect::getInstance()->setAlpha(timeRaito);

	const float mosCoe = powf(timeRaito, 5);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::getInstance()->getWindowSize().x * mosCoe,
													 WinAPI::getInstance()->getWindowSize().y * mosCoe));
}

void RailShoot::update_play() {
	debugText->Print(spriteBase.get(), "RailShoot", 0, 0);
	debugText->formatPrint(spriteBase.get(),
						   0, DebugText::fontHeight, 1.f, { 1,1,0,1 },
						   "%6.2f FPS", dxBase->getFPS());

	if (input->hitKey(DIK_LSHIFT) && input->hitKey(DIK_SPACE)) {
		changeNextScene();
	}

	// 敵を増やす
	const bool triggerEnter = input->triggerKey(DIK_RETURN);
	if (triggerEnter) {
		enemy.emplace_back(std::make_unique<Enemy>(camera.get(), enemyModel.get(), XMFLOAT3(0, -20, 300)));
		enemy.back()->setScale(5.f);
		enemy.back()->setVel(XMFLOAT3(0, 0, -1));
	}

	const bool hitW = input->hitKey(DIK_W);
	const bool hitA = input->hitKey(DIK_A);
	const bool hitS = input->hitKey(DIK_S);
	const bool hitD = input->hitKey(DIK_D);
	const bool hitE = input->hitKey(DIK_E);
	const bool hitQ = input->hitKey(DIK_Q);

	// 自機移動
	if (hitW || hitA || hitS || hitD) {
		XMFLOAT3 pPos = player->getPos();
		const float speed = 60.f / dxBase->getFPS();

		const XMFLOAT2 posSize = XMFLOAT2(WinAPI::getInstance()->getWindowSize().x * 0.12f,
										  WinAPI::getInstance()->getWindowSize().y * 0.12f);

		// 高さ方向に移動
		if (hitW && pPos.y < posSize.y) {
			pPos.y += speed;
		} else if (hitS && pPos.y > -posSize.y) {
			pPos.y -= speed;
		}
		// 横方向に移動
		if (hitA && pPos.x > -posSize.x) {
			pPos.x -= speed;
		} else if (hitD && pPos.x < posSize.x) {
			pPos.x += speed;
		}
		player->setPos(pPos);
	}
	// 自機回転
	if (hitE || hitQ) {
		const float speed = 90.f / dxBase->getFPS();

		XMFLOAT3 rota = player->getRotation();

		// y軸を回転軸とする回転
		if (hitE) {
			rota.y += speed;
		} else if (hitQ) {
			rota.y -= speed;
		}

		player->setRotation(rota);
	}

	// 弾発射
	if (input->triggerKey(DIK_SPACE)) {
		constexpr float bulSpeed = 8.f;
		player->shot(camera.get(), playerBulModel.get(), bulSpeed);
	}

	// 自機弾と敵の当たり判定
	{
		Sphere pBulCol{};
		for (auto &pb : player->getBulArr()) {
			if (!pb.getAlive()) continue;

			pBulCol.center = XMLoadFloat3(&pb.getPos());
			pBulCol.radius = pb.getScale();

			for (auto &e : enemy) {
				if (e->getAlive()
					&& Collision::CheckHit(pBulCol,
										   Sphere(XMLoadFloat3(&e->getPos()),
												  e->getScale()))) {
					// パーティクルを生成
					createParticle(e->getPos(), 98U, 32.f, 16.f);
					// 敵も自機弾もさよなら
					e->kill();
					pb.kill();
				}
			}
		}

		// 死んだ敵は消す
		enemy.erase(std::remove_if(enemy.begin(),
								   enemy.end(),
								   [](const std::unique_ptr<Enemy> &i) {return !i->getAlive(); }),
					enemy.end());

		// 敵がすべて消えたら次のシーンへ
		if (enemy.empty()) {
			changeNextScene();
		} else {
			debugText->formatPrint(spriteBase.get(),
								   0, DebugText::fontHeight * 2.f, 1.f,
								   { 1,1,1,1 },
								   "EnemyNum : %u", enemy.size());
		}
	}
}

void RailShoot::update_end() {
	const Time::timeType nowTime = timer->getNowTime() - startSceneChangeTime;
	if (nowTime >= sceneChangeTime) {
		SceneManager::getInstange()->changeScene(new PlayScene());
	}

	const float timeRaito = (float)nowTime / sceneChangeTime;
	PostEffect::getInstance()->setAlpha(1.f - timeRaito);

	const float mosCoe = powf(1.f - timeRaito, 5);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::window_width * mosCoe,
													 WinAPI::window_height * mosCoe));
}

void RailShoot::drawObj3d() {
	Object3d::startDraw(backPipelineSet);
	back->drawWithUpdate(light.get());

	Object3d::startDraw();
	player->drawWithUpdate(light.get());
	for (auto &i : enemy) {
		i->drawWithUpdate(light.get());
	}

	particleMgr->drawWithUpdate();
}

void RailShoot::drawFrontSprite() {
	spriteBase->drawStart(dxBase->getCmdList());
	debugText->DrawAll(dxBase, spriteBase.get());
}

RailShoot::~RailShoot() {
	PostEffect::getInstance()->setAlpha(1.f);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::window_width,
													 WinAPI::window_height));
	PostEffect::getInstance()->changePipeLine(0U);
}