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

	camera(std::make_unique<Camera>(WinAPI::window_width, WinAPI::window_height)),
	light(std::make_unique<Light>()),

	timer(std::make_unique<Time>()),

	spriteBase(std::make_unique<SpriteBase>()),

	back(std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true)),
	playerObj(std::make_unique<ObjSet>(camera.get(), "Resources/sphere", "sphere", false)),

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
	playerObj->setPos(XMFLOAT3(0, 0, 0));
	constexpr float playerScale = 10.f;
	playerObj->setScale(XMFLOAT3(playerScale, playerScale, playerScale));

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
		update_proc = std::bind(&RailShoot::update_play, this);
	}

	const float timeRaito = (float)nowTime / sceneChangeTime;
	PostEffect::getInstance()->setAlpha(timeRaito);
}

void RailShoot::update_play() {
	debugText->Print(spriteBase.get(), "RailShoot",
					 0, WinAPI::window_height / 2.f, 5.f);

	if (input->hitKey(DIK_SPACE)) {
		update_proc = std::bind(&RailShoot::update_end, this);
		startSceneChangeTime = timer->getNowTime();
	}


	const bool hitW = input->hitKey(DIK_W);
	const bool hitA = input->hitKey(DIK_A);
	const bool hitS = input->hitKey(DIK_S);
	const bool hitD = input->hitKey(DIK_D);
	const bool hitE = input->hitKey(DIK_E);
	const bool hitZ = input->hitKey(DIK_Z);

	if (hitW || hitA || hitS || hitD || hitE || hitZ) {
		XMFLOAT3 pPos = playerObj->getPos();
		constexpr float speed = 1.f;

		if (hitW) {
			pPos.y += speed;
		} else if (hitS) {
			pPos.y -= speed;
		}
		if (hitA) {
			pPos.x -= speed;
		} else if (hitD) {
			pPos.x += speed;
		}
		if (hitE) {
			pPos.z += speed;
		} else if (hitZ) {
			pPos.z -= speed;
		}

		playerObj->setPos(pPos);
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
	playerObj->drawWithUpdate(light.get());

}

void RailShoot::drawFrontSprite() {
	spriteBase->drawStart(dxBase->getCmdList());
	debugText->DrawAll(dxBase, spriteBase.get());
}

RailShoot::~RailShoot() {
	PostEffect::getInstance()->setAlpha(1.f);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::window_width,
													 WinAPI::window_height));
}