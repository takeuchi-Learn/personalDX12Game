﻿#include "PlayScene.h"

#include "SceneManager.h"

#include <sstream>
#include <iomanip>

#include <xaudio2.h>
#include "RandomNum.h"

#include "FbxLoader.h"

#include "FbxObj3d.h"

#include "EndScene.h"

#include "PostEffect.h"

#include "Collision.h"

using namespace DirectX;

#pragma region 初期化関数

void PlayScene::cameraInit() {
	camera.reset(new Camera(WinAPI::window_width, WinAPI::window_height));
	camera->setFarZ(5000.f);
	camera->setEye(XMFLOAT3(0, 0, -175));	// 視点座標
	camera->setTarget(XMFLOAT3(0, 0, 0));	// 注視点座標
	camera->setUp(XMFLOAT3(0, 1, 0));		// 上方向
	camera->update();
}

void PlayScene::lightInit() {
	light.reset(new Light());
}

void PlayScene::soundInit() {
	soundBase.reset(new SoundBase());
	bgm.reset(new Sound("Resources/BGM.wav", soundBase.get()));

	particleSE.reset(new Sound("Resources/SE/Sys_Set03-click.wav", soundBase.get()));
}

void PlayScene::spriteInit() {


	// --------------------
	// スプライト共通
	// --------------------
	spriteBase.reset(new SpriteBase());

	// スプライト共通テクスチャ読み込み
	//texNum = spriteBase->loadTexture(L"Resources/texture.png");

	//// スプライトの生成
	//sprites.resize(SPRITES_NUM);
	//for (UINT i = 0; i < SPRITES_NUM; ++i) {
	//	sprites[i] = Sprite(texNum, spriteBase.get(), { 0, 0 });
	//	// スプライトの座標変更
	//	sprites[i].position.x = 1280.f / 10;
	//	sprites[i].position.y = 720.f / 10;
	//	//sprites[i].isInvisible = true;
	//	//sprites[i].position.x = (float)(rand() % 1280);
	//	//sprites[i].position.y = (float)(rand() % 720);
	//	//sprites[i].rotation = (float)(rand() % 360);
	//	//sprites[i].rotation = 0;
	//	//sprites[i].size.x = 400.0f;
	//	//sprites[i].size.y = 100.0f;
	//}

	// デバッグテキスト用のテクスチャ読み込み
	debugTextTexNumber = spriteBase->loadTexture(L"Resources/debugfont.png");
	// デバッグテキスト初期化
	debugText.reset(new DebugText(debugTextTexNumber, spriteBase.get()));


}

void PlayScene::obj3dInit() {

	// 3Dオブジェクト用パイプライン生成
	object3dPipelineSet = Object3d::createGraphicsPipeline(dxBase->getDev());

	backPipelineSet = Object3d::createGraphicsPipeline(dxBase->getDev(), Object3d::BLEND_MODE::ALPHA,
													   L"Resources/Shaders/BackVS.hlsl",
													   L"Resources/Shaders/BackPS.hlsl");


	// ----------
	// 天球
	// ----------
	{
		back = std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true);
		const float backScale = camera->getFarZ() * 0.9f;
		back->setScale({ backScale, backScale, backScale });
	}



	// ----------
	// ボス
	// ----------
	{
		bossTimer = std::make_unique<Time>();

		boss = std::make_unique<ObjSet>(camera.get(), "Resources/sphere", "sphere", true);

		constexpr float bossScale = 100.f;
		boss->setScale(XMFLOAT3(bossScale, bossScale, bossScale));
		boss->setPos(XMFLOAT3(0, bossScale, 0));
	}

	// ----------
	// 自機弾
	// ----------
	{
		playerBul.first = std::make_unique<ObjSet>(camera.get(), "Resources/sphere", "sphere", true);
		playerBul.second = false;
		constexpr float pBulScale = 10.f;
		playerBul.first->setScale(XMFLOAT3(pBulScale, pBulScale, pBulScale));

		playerBulTimer = std::make_unique<Time>();
	}

	// ----------
	// 地面
	// ----------
	{
		ground = std::make_unique<ObjSet>(camera.get(), "Resources/ground", "ground");
		ground->setPos(XMFLOAT3(0, -playerBul.first->getScale().y, 0));
		const float groundScale = camera->getFarZ();
		ground->setScale(XMFLOAT3(groundScale, groundScale, groundScale));
	}
}

void PlayScene::fbxInit() {
	FbxObj3d::setDevice(dxBase->getDev());
	FbxObj3d::setCamera(camera.get());
	FbxObj3d::createGraphicsPipeline(L"Resources/Shaders/FBXVS.hlsl",
									 L"Resources/Shaders/FBXPS.hlsl");

	constexpr char fbxName[] = "player";
	playerFbxModel.reset(FbxLoader::GetInstance()->loadModelFromFile(fbxName));

	/*playerFbxModel->setAmbient(XMFLOAT3(0.5f, 0.5f, 0.5f));
	playerFbxModel->setSpecular(XMFLOAT3(0.8f, 0.8f, 0.8f));*/

	playerFbxObj3d.reset(new FbxObj3d(playerFbxModel.get()/*, false*/));
	const float fbxObjScale = 0.125f;
	playerFbxObj3d->setScale(XMFLOAT3(fbxObjScale, fbxObjScale, fbxObjScale));
	playerFbxObj3d->setPosition(player->getPosF3());
}

void PlayScene::particleInit() {
	particleMgr.reset(new ParticleMgr(L"Resources/effect1.png", camera.get()));
}
void PlayScene::playerInit() {
	player = std::make_unique<Player>();
	player->setPos(XMFLOAT3(0, 10, -300.f));
}

void PlayScene::timerInit() {
	timer.reset(new Time());
}

#pragma endregion 初期化関数

PlayScene::PlayScene()
	: update_proc(std::bind(&PlayScene::update_start, this)) {

	dxBase = DX12Base::getInstance();

	input = Input::getInstance();

	postEff2Num = (UINT)PostEffect::getInstance()->addPipeLine(L"Resources/Shaders/PostEffectPS_2.hlsl");


	cameraInit();

	lightInit();

	soundInit();

	spriteInit();

	playerInit();

	obj3dInit();

	fbxInit();

	particleInit();

	timerInit();
}

void PlayScene::init() {
	// BGM再生
	Sound::SoundPlayWave(soundBase.get(), bgm.get(), XAUDIO2_LOOP_INFINITE);
	// タイマー開始
	timer->reset();
}

#pragma region 更新関数

void PlayScene::updateSound() {
	// 数字の0キーが押された瞬間音を再生しなおす
	if (input->triggerKey(DIK_0)) {
		//Sound::SoundStopWave(bgm);

		if (Sound::checkPlaySound(bgm.get())) {
			Sound::SoundStopWave(bgm.get());
		} else {
			Sound::SoundPlayWave(soundBase.get(), bgm.get(), XAUDIO2_LOOP_INFINITE);
		}
	}
}

void PlayScene::updateMouse() {
	constexpr XMFLOAT2 centerPos = XMFLOAT2((float)WinAPI::window_width / 2.f,
											(float)WinAPI::window_height / 2.f);

	// 中心からの距離
	const XMFLOAT2 mousePos(float(input->getMousePos().x) - centerPos.x,
							float(input->getMousePos().y) - centerPos.y);

	const float camMoveVel = 0.125f / dxBase->getFPS();

	cameraMoveVel.x += camMoveVel * mousePos.x;
	cameraMoveVel.y += camMoveVel * mousePos.y;

	input->setMousePos((int)centerPos.x, (int)centerPos.y);
}

void PlayScene::updateCamera() {
	// カメラの距離
	constexpr float camLen = 64.f;
	// カメラの高さ
	constexpr float camHeight = camLen * 0.5f;

	// 自機からカメラ注視点までの距離
	constexpr float player2targetLen = camLen * 2.f;

	// 自機の視線ベクトル
	const XMVECTOR look = XMVector3Rotate(XMVector3Normalize(player->getLookVec()),
										  XMQuaternionRotationRollPitchYaw(cameraMoveVel.y,
																		   cameraMoveVel.x,
																		   0.f));

	// 自機->カメラのベクトル
	const XMVECTOR player2cam = XMVectorAdd(XMVectorScale(look, -camLen),
											XMVectorSet(0, camHeight, 0, 1));

	// カメラの位置
	{
		const XMVECTOR pos = XMVectorAdd(player->getPosVec(), player2cam);

		XMFLOAT3 camPos{};
		XMStoreFloat3(&camPos, pos);

		camera->setEye(camPos);
	}

	// 注視点設定
	{
		const XMVECTOR targetPos = XMVectorAdd(XMVectorScale(look, player2targetLen),
											   player->getPosVec());
		XMFLOAT3 targetF3{};
		XMStoreFloat3(&targetF3, targetPos);

		camera->setTarget(targetF3);
	}
}

void PlayScene::updateLight() {
	XMFLOAT3 pos = camera->getEye();
	//pos.y += playerFbxObj3d->getScale().y * 3.f;

	light->setLightPos(pos);
}

void PlayScene::updateSprite() {

}

void PlayScene::updatePlayer() {

	// 移動
	{
		const bool hitW = input->hitKey(DIK_W);
		const bool hitA = input->hitKey(DIK_A);
		const bool hitS = input->hitKey(DIK_S);
		const bool hitD = input->hitKey(DIK_D);

		if (hitW || hitA || hitS || hitD) {
			// 移動速度は毎秒60
			const float moveVel = 60.f / dxBase->getFPS();

			if (hitW) {
				player->moveForward(moveVel);
			} else if (hitS) {
				player->moveForward(-moveVel);
			}

			if (hitA) {
				player->moveRight(-moveVel);
			} else if (hitD) {
				player->moveRight(moveVel);
			}
		}

		playerFbxObj3d->setPosition(player->getPosF3());
	}

	// 弾発射
	{
		const bool triggerSpace = input->triggerKey(DIK_SPACE);

		// playerBul.secondは生存フラグ

		if (triggerSpace && !playerBul.second) {
			playerBul.second = true;
			playerBul.first->setPos(camera->getEye());
			// todo 自機から出るようにする
			//playerBul.first->setPos(player->getPosF3());

			playerBulVel = camera->getLook();
			//XMStoreFloat3(&playerBulVel, player->getLookVec());

			playerBulTimer->reset();
		}
	}

	// 敵との衝突
	{
		Sphere bossSphere{};
		bossSphere.center = XMLoadFloat3(&boss->getPos());
		bossSphere.radius = boss->getScale().x;

		Sphere playerSphere{};
		playerSphere.center = XMLoadFloat3(&camera->getEye());
		playerSphere.radius = playerFbxObj3d->getScale().x;

		const bool hitBoss = Collision::CheckSphere2Sphere(playerSphere, bossSphere);

		// 自機がボスに当たったら終了
		if (hitBoss) {
			changeEndScene();
		}
	}
}

void PlayScene::updatePlayerBullet() {
	if (playerBul.second) {
		XMFLOAT3 pos = playerBul.first->getPos();

		constexpr float speed = 10.f;

		pos.x += playerBulVel.x * speed;
		pos.y += playerBulVel.y * speed;
		pos.z += playerBulVel.z * speed;

		playerBul.first->setPos(pos);

		{
			Sphere pBul{};
			pBul.center = XMLoadFloat3(&pos);
			pBul.radius = playerBul.first->getScale().x;


			Sphere bossCol{};
			bossCol.center = XMLoadFloat3(&boss->getPos());
			bossCol.radius = boss->getScale().x;

			Plane gr{};
			gr.normal = XMVectorSet(0, 1, 0, 1);
			gr.distance = Collision::vecLength(XMLoadFloat3(&ground->getPos()));

			const bool hitBoss = Collision::CheckSphere2Sphere(pBul, bossCol);

			const bool hitGround = Collision::CheckSphere2Plane(pBul, gr);
			debugText->formatPrint(spriteBase.get(), 300, 0, 1.f, { 1,1,1,1 },
								   "%s", hitGround ? "true" : "false");

			const bool lifeEnd = playerBulTimer->getNowTime() > Time::oneSec;

			if (hitBoss || hitGround || lifeEnd) {
				playerBul.second = false;

				// 弾がボスに当たったら終了
				if (hitBoss) {
					// ボスの描画をやめる
					bossAlive = false;

					// パーティクル発生
					createParticle(boss->getPos(), 256U, 64.f, 32.f);

					// SE鳴らす
					Sound::SoundPlayWave(soundBase.get(), particleSE.get());

					// 次のシーンへ
					changeEndScene();
				}
			}
		}
	}
}

void PlayScene::updateBoss() {
	const float moveRange = boss->getScale().x * 5.f;
	XMFLOAT3 bossPos = boss->getPos();
	bossPos.x = dxBase->nearSin(bossTimer->getNowTime() / Time::oneSecF * XM_PIDIV2) * moveRange;
	bossPos.z = dxBase->nearCos(bossTimer->getNowTime() / Time::oneSecF * XM_1DIVPI) * moveRange;
	boss->setPos(bossPos);
}

void PlayScene::update_play() {

	// SPACE + 左シフトでENDシーンへ
	if (input->triggerKey(DIK_SPACE) && input->hitKey(DIK_LSHIFT)) {
		changeEndScene();
	}

	// Rでタイマーをリセット
	if (input->hitKey(DIK_R)) timer->reset();

	if (input->triggerKey(DIK_M)) {
		UINT nextEffNum = 0u;
		if (PostEffect::getInstance()->getPipeLineNum() == 0u) {
			nextEffNum = postEff2Num;
		}
		PostEffect::getInstance()->changePipeLine(nextEffNum);
	}


	// 音関係の更新
	updateSound();

	// マウス情報の更新
	updateMouse();

	// 自機更新
	updatePlayer();

	// ボス更新
	updateBoss();

	// 自機弾更新
	updatePlayerBullet();

	// カメラの更新
	updateCamera();

	// ライトの更新
	updateLight();

	// スプライトの更新
	updateSprite();

#pragma region 情報表示

	//if (input->triggerKey(DIK_T)) {
	//	debugText->tabSize++;
	//	if (input->hitKey(DIK_LSHIFT)) debugText->tabSize = 4U;
	//}


	//debugText->formatPrint(spriteBase.get(),
	//					   DebugText::fontWidth * 2.f, DebugText::fontHeight * 17.f,
	//					   1.f,
	//					   XMFLOAT4(1, 1, 1, 1),
	//					   "newLine\ntab(size %u)\tendString", debugText->tabSize);

#pragma endregion 情報表示
}

#pragma endregion 更新関数

void PlayScene::update() {
	{
		// シーン遷移中も背景は回す
		XMFLOAT3 backRota = back->getRotation();
		backRota.y += 6.f / dxBase->getFPS();
		back->setRotation(backRota);
	}

	// 主な処理
	update_proc();

	// 背景オブジェクトの中心をカメラにする
	back->setPos(camera->getEye());
	{
		XMFLOAT3 gPos = ground->getPos();
		gPos.x = back->getPos().x;
		gPos.z = back->getPos().z;
		ground->setPos(gPos);
	}

	// ライトとカメラの更新
	light->update();
	camera->update();
}

// シーン開始時の演出用
void PlayScene::update_start() {
	drawAlpha = (float)timer->getNowTime() / sceneTransTime;

	if (drawAlpha > 1.f) {
		drawAlpha = 1.f;

		playerFbxObj3d->playAnimation();
		timer->reset();

		update_proc = std::bind(&PlayScene::update_play, this);
	}
	PostEffect::getInstance()->setAlpha(drawAlpha);

	// drawAlphaを基準としたモザイクでの入り
	constexpr XMFLOAT2 mosNumMin{ 1.f, 1.f };
	constexpr XMFLOAT2 mosNumMax{ WinAPI::window_width, WinAPI::window_height };
	const float mosRaito = powf(drawAlpha, 5);
	XMFLOAT2 mosNum = mosNumMax;

	mosNum.x = mosNumMin.x + mosRaito * (mosNumMax.x - mosNumMin.x);
	mosNum.y = mosNumMin.y + mosRaito * (mosNumMax.y - mosNumMin.y);

	PostEffect::getInstance()->setMosaicNum(mosNum);
}

// シーン終了時の演出用
void PlayScene::update_end() {
	const float raito = (float)timer->getNowTime() / sceneTransTime;

	drawAlpha = 1.f - raito;
	if (raito > 1.f) {
		drawAlpha = 0.f;
		SceneManager::getInstange()->changeScene(new EndScene());
	}

	PostEffect::getInstance()->setAlpha(drawAlpha);

	// drawAlphaを基準としたモザイク
	constexpr XMFLOAT2 mosNumMin{ 1.f, 1.f };
	constexpr XMFLOAT2 mosNumMax{ WinAPI::window_width, WinAPI::window_height };

	const float mosRaito = powf(1.f - raito, 5);

	XMFLOAT2 mosNum = mosNumMax;
	mosNum.x = mosNumMin.x + mosRaito * (mosNumMax.x - mosNumMin.x);
	mosNum.y = mosNumMin.y + mosRaito * (mosNumMax.y - mosNumMin.y);

	PostEffect::getInstance()->setMosaicNum(mosNum);
}

void PlayScene::changeEndScene() {
	// BGMが鳴っていたら停止する
	if (Sound::checkPlaySound(bgm.get())) {
		Sound::SoundStopWave(bgm.get());
	}

	/*for (Sprite &i : sprites) {
		i.isInvisible = true;
	}*/

	// fbxのアニメーションを停止する
	playerFbxObj3d->stopAnimation(false);

	drawAlpha = 1.f;
	PostEffect::getInstance()->setAlpha(drawAlpha);

	update_proc = std::bind(&PlayScene::update_end, this);
	timer->reset();
}

void PlayScene::drawObj3d() {

	Object3d::startDraw(dxBase->getCmdList(), backPipelineSet);
	back->drawWithUpdate(light.get());

	ParticleMgr::startDraw(dxBase->getCmdList(), object3dPipelineSet);
	particleMgr->drawWithUpdate(dxBase->getCmdList());

	Object3d::startDraw(dxBase->getCmdList(), object3dPipelineSet);
	ground->drawWithUpdate(light.get());
	if (bossAlive) boss->drawWithUpdate(light.get());
	if (playerBul.second) playerBul.first->drawWithUpdate(light.get());

	// 自機描画
	//playerFbxObj3d->drawWithUpdate(dxBase->getCmdList(), light.get());
}

void PlayScene::drawFrontSprite() {
	drawImGui();

	spriteBase->drawStart(dxBase->getCmdList());
	// スプライト描画
	/*for (UINT i = 0, len = (UINT)sprites.size(); i < len; ++i) {
		sprites[i].drawWithUpdate(dxBase, spriteBase.get());
	}*/

	// デバッグテキスト描画
	debugText->DrawAll(dxBase, spriteBase.get());
}

void PlayScene::drawImGui() {
	constexpr ImGuiWindowFlags winFlags =
		// リサイズ不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize
		// タイトルバー無し
		//| ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar
		// 設定を.iniに出力しない
		| ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings
		// 移動不可
		| ImGuiWindowFlags_::ImGuiWindowFlags_NoMove;

	// 最初のウインドウの位置を指定
	ImGui::SetNextWindowPos(ImVec2(20, 20));


	ImGui::Begin("内容説明", nullptr, winFlags);
	ImGui::Text("弾をでかい球体に当てたらクリア");
	ImGui::Text("でかい球と自分が当たったら失敗");
	ImGui::Text("弾は地面に当たるか時間経過で消滅！！");
	ImGui::SetNextWindowPos(getWindowLBPos());
	ImGui::End();

	if (guiWinAlive) {
		ImGui::Begin("情報表示", &guiWinAlive, winFlags);
		//ImGui::SetWindowPos(ImVec2(20, 20));
		//ImGui::SetWindowSize(ImVec2(300, 300));
		ImGui::Text("FPS <- %.3f", dxBase->getFPS());
		ImGui::Text("時間 <- %.6f秒",
					float(timer->getNowTime()) / float(Time::oneSec));
		ImGui::Text("BGM再生状態 <- %s",
					Sound::checkPlaySound(bgm.get())
					? "再生|>"
					: "停止[]");
		ImGui::Text("ホイール%d", input->getInstance()->getMouseWheelScroll());
		// 次のウインドウは今のウインドウのすぐ下
		ImGui::SetNextWindowPos(getWindowLBPos());
		ImGui::End();
	}

	ImGui::Begin("操作説明", nullptr, winFlags);
	ImGui::Text("0 : BGM再生/停止");
	ImGui::Text("SPACE : 弾発射");
	ImGui::Text("左シフト + SPACE : 終了");
	ImGui::Text("WASD : 視線方向に移動");
	ImGui::Text("マウス : カメラ回転");
	ImGui::Text("M : シェーダー変更");
	ImGui::SetNextWindowPos(getWindowLBPos());
	ImGui::End();
}

void PlayScene::createParticle(const DirectX::XMFLOAT3 &pos,
							   const UINT particleNum,
							   const float startScale,
							   const float vel) {
	for (UINT i = 0U; i < particleNum; ++i) {

		const float theata = RandomNum::getRandf(0, XM_PI);
		const float phi = RandomNum::getRandf(0, XM_PI * 2.f);
		const float r = RandomNum::getRandf(0, vel);

		XMFLOAT3 generatePos = pos;

		const XMFLOAT3 vel{
			r * dxBase->nearSin(theata) * dxBase->nearCos(phi),
			r * dxBase->nearCos(theata),
			r * dxBase->nearSin(theata) * dxBase->nearSin(phi)
		};

		XMFLOAT3 acc{};


		constexpr XMFLOAT3 startCol = XMFLOAT3(1, 1, 0.25f), endCol = XMFLOAT3(1, 0, 1);
		constexpr int life = Time::oneSec / 4;
		constexpr float endScale = 0.f;
		constexpr float startRota = 0.f, endRota = 0.f;

		particleTimer.reset(new Time());

		// 追加
		particleMgr->add(std::move(particleTimer),
						 life, generatePos, vel, acc,
						 startScale, endScale,
						 startRota, endRota,
						 startCol, endCol);
	}
}

PlayScene::~PlayScene() {
	//Sound::SoundStopWave(bgm.get());
}