#include "RailShoot.h"
#include <DirectXMath.h>

#include "SceneManager.h"
#include "PlayScene.h"

#include "PostEffect.h"

using namespace DirectX;

const Time::timeType RailShoot::sceneChangeTime = Time::oneSec;

RailShoot::RailShoot()
	: dxBase(DX12Base::getInstance()),
	input(Input::getInstance()),

	update_proc(std::bind(&RailShoot::update_start, this)),

	camera(std::make_unique<Camera>(WinAPI::WinAPI::getInstance()->getWindowSIze().x, WinAPI::getInstance()->getWindowSIze().y)),
	light(std::make_unique<Light>()),

	timer(std::make_unique<Time>()),

	spriteBase(std::make_unique<SpriteBase>()),

	back(std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true)),
	player(std::make_unique<Player>(camera.get(), "Resources/box", "box", false)),

	playerBulModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),

	startSceneChangeTime(0U),

	object3dPipelineSet(Object3d::createGraphicsPipeline(dxBase->getDev())),
	backPipelineSet(Object3d::createGraphicsPipeline(dxBase->getDev(),
													 Object3d::BLEND_MODE::ALPHA,
													 L"Resources/Shaders/BackVS.hlsl",
													 L"Resources/Shaders/BackPS.hlsl")) {
	// スプライト初期化
	const UINT debugTextTexNumber = spriteBase->loadTexture(L"Resources/debugfont.png");
	debugText = std::make_unique<DebugText>(debugTextTexNumber, spriteBase.get());

	// カメラ初期化
	camera->setFarZ(5000.f);
	camera->setEye(XMFLOAT3(0, 0, -175));	// 視点座標
	camera->setTarget(XMFLOAT3(0, 0, 0));	// 注視点座標
	camera->setUp(XMFLOAT3(0, 1, 0));		// 上方向
	camera->update();

	// ライト初期化
	light->setLightPos(camera->getEye());

	// 自機初期化
	player->setPos(XMFLOAT3(0, 0, 0));
	constexpr float playerScale = 10.f;
	player->setScale(XMFLOAT3(playerScale, playerScale, playerScale));

	// 天球
	const float backScale = camera->getFarZ() * 0.9f;
	back->setScale({ backScale, backScale, backScale });

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

void RailShoot::update_start() {
	const Time::timeType nowTime = timer->getNowTime() - startSceneChangeTime;
	if (nowTime >= sceneChangeTime) {
		PostEffect::getInstance()->changePipeLine(SceneManager::getInstange()->getPostEff2Num());

		update_proc = std::bind(&RailShoot::update_play, this);
	}

	const float timeRaito = (float)nowTime / sceneChangeTime;
	PostEffect::getInstance()->setAlpha(timeRaito);

	const float mosCoe = powf(timeRaito, 5);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::getInstance()->getWindowSIze().x * mosCoe,
													 WinAPI::getInstance()->getWindowSIze().y * mosCoe));
}

void RailShoot::update_play() {
	debugText->Print(spriteBase.get(), "RailShoot", 0, 0);

	if (input->hitKey(DIK_LSHIFT) && input->hitKey(DIK_SPACE)) {
		PostEffect::getInstance()->changePipeLine(0U);

		update_proc = std::bind(&RailShoot::update_end, this);
		startSceneChangeTime = timer->getNowTime();
	}


	const bool hitW = input->hitKey(DIK_W);
	const bool hitA = input->hitKey(DIK_A);
	const bool hitS = input->hitKey(DIK_S);
	const bool hitD = input->hitKey(DIK_D);
	const bool hitE = input->hitKey(DIK_E);
	const bool hitQ = input->hitKey(DIK_Q);

	if (hitW || hitA || hitS || hitD) {
		XMFLOAT3 pPos = player->getPos();
		const float speed = 60.f / dxBase->getFPS();

		// 高さ方向に移動
		if (hitW && pPos.y < WinAPI::window_height * 0.12f) {
			pPos.y += speed;
		} else if (hitS && pPos.y > -WinAPI::window_height * 0.12f) {
			pPos.y -= speed;
		}
		// 横方向に移動
		if (hitA && pPos.x > -WinAPI::window_width * 0.125f) {
			pPos.x -= speed;
		} else if (hitD && pPos.x < WinAPI::window_width * 0.125f) {
			pPos.x += speed;
		}
		player->setPos(pPos);
	}
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
		player->shot(camera.get(), playerBulModel.get(), XMFLOAT3(0, 0, bulSpeed));
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
	Object3d::startDraw(dxBase->getCmdList(), backPipelineSet);
	back->drawWithUpdate(light.get());

	Object3d::startDraw(dxBase->getCmdList(), object3dPipelineSet);
	player->drawWithUpdate(light.get());

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