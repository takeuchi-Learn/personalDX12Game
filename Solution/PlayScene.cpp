#include "PlayScene.h"

#include "SceneManager.h"

#include <sstream>
#include <iomanip>

#include <xaudio2.h>
#include "RandomNum.h"

#include "FbxLoader.h"

#include "FbxObj3d.h"

#include "EndScene.h"

#include "PostEffect.h"

using namespace DirectX;

#pragma region 角度系関数

float PlayScene::angleRoundRad(float rad) {
	float angle = rad;

	if (angle >= 0.f && angle < XM_2PI) return angle;

	while (angle >= XM_2PI) {
		angle -= XM_2PI;
	}
	while (angle < 0) {
		angle += XM_2PI;
	}
	return angle;
}

float PlayScene::nearSin(float rad) {
	constexpr float a = +0.005859483f;
	constexpr float b = +0.005587939f;
	constexpr float c = -0.171570726f;
	constexpr float d = +0.0018185485f;
	constexpr float e = +0.9997773594f;

	float x = angleRoundRad(rad);

	// 0 ~ PI/2がわかれば求められる
	if (x < XM_PIDIV2) {
		// そのまま
	} else if (x >= XM_PIDIV2 && x < XM_PI) {
		x = XM_PI - x;
	} else if (x < XM_PI * 1.5f) {
		x = -(x - XM_PI);
	} else if (x < XM_2PI) {
		x = -(XM_2PI - x);
	}

	return x * (x * (x * (x * (a * x + b) + c) + d) + e);
}

float PlayScene::nearCos(float rad) {
	return nearSin(rad + XM_PIDIV2);
}

float PlayScene::nearTan(float rad) {
	return nearSin(rad) / nearCos(rad);
}

double PlayScene::near_atan2(double _y, double _x) {

	const double x = abs(_x);
	const double y = abs(_y);

	const bool bigX = y < x;

	double slope{};
	if (bigX) slope = (double)y / x;
	else  slope = (double)x / y;

	constexpr double a = -0.05026472;
	constexpr double b = +0.26603324;
	constexpr double c = -0.45255286;
	constexpr double d = +0.02385002;
	constexpr double e = +0.99836359;

	double ret = slope * (slope * (slope * (slope * (a * slope + b) + c) + d) + e); //5次曲線近似

	constexpr float plane = XM_PI;
	constexpr float rightAngle = plane / 2.f;	// 直角

	if (bigX) {
		if (_x > 0) {
			if (_y < 0) ret = -ret;
		} else {
			if (_y > 0) ret = plane - ret;
			if (_y < 0) ret = ret - plane;
		}
	} else {
		if (_x > 0) {
			if (_y > 0) ret = rightAngle - ret;
			if (_y < 0) ret = ret - rightAngle;
		}
		if (_x < 0) {
			if (_y > 0) ret = ret + rightAngle;
			if (_y < 0) ret = -ret - rightAngle;
		}
	}

	return ret;
}

float PlayScene::near_atan2(float y, float x) {
	return (float)near_atan2((double)y, (double)x);
}

#pragma endregion 角度系関数

#pragma region 初期化関数

void PlayScene::cameraInit() {
	camera.reset(new Camera(WinAPI::window_width, WinAPI::window_height));
	camera->setFarZ(10000.f);
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
	soundData1.reset(new Sound("Resources/BGM.wav", soundBase.get()));

	particleSE.reset(new Sound("Resources/SE/Sys_Set03-click.wav", soundBase.get()));
}

void PlayScene::spriteInit() {


	// --------------------
	// スプライト共通
	// --------------------
	spriteBase.reset(new SpriteBase());

	// スプライト共通テクスチャ読み込み
	texNum = spriteBase->loadTexture(L"Resources/texture.png");

	// スプライトの生成
	sprites.resize(SPRITES_NUM);
	for (UINT i = 0; i < SPRITES_NUM; ++i) {
		sprites[i] = Sprite(texNum, spriteBase.get(), { 0, 0 });
		// スプライトの座標変更
		sprites[i].position.x = 1280.f / 10;
		sprites[i].position.y = 720.f / 10;
		//sprites[i].isInvisible = true;
		//sprites[i].position.x = (float)(rand() % 1280);
		//sprites[i].position.y = (float)(rand() % 720);
		//sprites[i].rotation = (float)(rand() % 360);
		//sprites[i].rotation = 0;
		//sprites[i].size.x = 400.0f;
		//sprites[i].size.y = 100.0f;
	}

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



	backModel.reset(new ObjModel("Resources/back/", "back", 1u, true));

	backObj.reset(new Object3d(DX12Base::getInstance()->getDev(), camera.get(), backModel.get(), 1u));
	constexpr float backScale = 10.f;
	backObj->scale = { backScale, backScale, backScale };

	/*model.reset(new ObjModel(DX12Base::getInstance()->getDev(),
						  L"Resources/model/model.obj", L"Resources/model/tex.png",
						  WinAPI::window_width, WinAPI::window_height,
						  Object3d::constantBufferNum, obj3dTexNum));*/
	model.reset(new ObjModel("Resources/model/", "model", obj3dTexNum, true));

	constexpr UINT obj3dNum = 1;
	for (UINT i = 0; i < obj3dNum; ++i) {
		obj3d.emplace_back(Object3d(DX12Base::getInstance()->getDev(), camera.get(), model.get(), obj3dTexNum));
		obj3d[i].scale = { obj3dScale, obj3dScale, obj3dScale };
		obj3d[i].position = { i * obj3dScale, 0, 0 };
		//obj3d[i].rotation.y = 180.f;
	}

	lightObj.reset(new Object3d(Object3d(DX12Base::getInstance()->getDev(), camera.get(), model.get(), obj3dTexNum)));
	const float lightObjScale = obj3dScale * 0.5f;
	lightObj->scale = XMFLOAT3(lightObjScale, lightObjScale, lightObjScale);
	lightObj->position = obj3d[0].position;
}

void PlayScene::fbxInit() {
	FbxObj3d::setDevice(dxBase->getDev());
	FbxObj3d::setCamera(camera.get());
	FbxObj3d::createGraphicsPipeline();

	constexpr char fbxName[] = "boneTest";
	fbxModel.reset(FbxLoader::GetInstance()->loadModelFromFile(fbxName));

	/*fbxModel->setAmbient(XMFLOAT3(0.5f, 0.5f, 0.5f));
	fbxModel->setSpecular(XMFLOAT3(0.8f, 0.8f, 0.8f));*/

	fbxObj3d.reset(new FbxObj3d(fbxModel.get()/*, false*/));
	constexpr float fbxObjScale = 0.0725f;
	fbxObj3d->setScale(XMFLOAT3(fbxObjScale, fbxObjScale, fbxObjScale));
}

void PlayScene::particleInit() {
	particleMgr.reset(new ParticleMgr(L"Resources/effect1.png", camera.get()));
}

void PlayScene::timerInit() {
	timer.reset(new Time());
}

#pragma endregion 初期化関数

PlayScene::PlayScene()
	: update_proc(std::bind(&PlayScene::update_start, this)) {
	WinAPI::getInstance()->setWindowText("Press SPACE to change scene - now : Play (SE : OtoLogic)");

	dxBase = DX12Base::getInstance();

	input = Input::getInstance();

	postEff2Num = (UINT)PostEffect::getInstance()->addPipeLine(L"Resources/Shaders/PostEffectPS_2.hlsl");


	cameraInit();

	lightInit();

	soundInit();

	spriteInit();

	obj3dInit();

	fbxInit();

	particleInit();

	timerInit();
}

void PlayScene::init() {
	// タイマー開始
	timer->reset();
}

#pragma region 更新関数

void PlayScene::updateSound() {
	// 数字の0キーが押された瞬間音を再生しなおす
	if (input->triggerKey(DIK_0)) {
		//Sound::SoundStopWave(soundData1);

		if (Sound::checkPlaySound(soundData1.get())) {
			Sound::SoundStopWave(soundData1.get());
		} else {
			Sound::SoundPlayWave(soundBase.get(), soundData1.get(), XAUDIO2_LOOP_INFINITE);
		}
	}
}

void PlayScene::updateMouse() {
	const XMFLOAT2 mousePos(float(input->getMousePos().x), float(input->getMousePos().y));

	if (input->hitMouseBotton(Input::MOUSE::LEFT)) {
		debugText->Print(spriteBase.get(), "input mouse left",
						 mousePos.x, mousePos.y, 0.75f);
	}
	if (input->hitMouseBotton(Input::MOUSE::RIGHT)) {
		debugText->Print(spriteBase.get(), "input mouse right",
						 mousePos.x,
						 mousePos.y + DebugText::fontHeight, 0.75f);
	}
	if (input->hitMouseBotton(Input::MOUSE::WHEEL)) {
		debugText->Print(spriteBase.get(), "input mouse wheel",
						 mousePos.x,
						 mousePos.y + DebugText::fontHeight * 2, 0.75f);
	}
	if (input->hitMouseBotton(VK_LSHIFT)) {
		debugText->Print(spriteBase.get(), "LSHIFT(WinAPI)", 0, 0, 2);
	}

	// Rを押すたびマウスカーソルの表示非表示を切り替え
	if (input->triggerKey(DIK_R)) {
		static bool mouseDispFlag = true;
		mouseDispFlag = !mouseDispFlag;
		input->changeDispMouseCursorFlag(mouseDispFlag);
	}

	// Mキーでマウスカーソル位置を0,0に移動
	if (input->triggerKey(DIK_M)) {
		input->setMousePos(0, 0);
	}
}

void PlayScene::updateCamera() {
	constexpr float PIDIV3 = XM_PI / 3.f;

	// 毎秒1/4周
	const float rotaVal = XM_PIDIV2 / DX12Base::getInstance()->getFPS();

	if (input->hitKey(DIK_RIGHT)) {
		angle.y += rotaVal;
		if (angle.y > XM_PI * 2) { angle.y = 0; }
	} else if (input->hitKey(DIK_LEFT)) {
		angle.y -= rotaVal;
		if (angle.y < 0) { angle.y = XM_PI * 2; }
	}

	if (input->hitKey(DIK_UP)) {
		if (angle.x + rotaVal < PIDIV3) angle.x += rotaVal;
	} else if (input->hitKey(DIK_DOWN)) {
		if (angle.x - rotaVal > -PIDIV3) angle.x -= rotaVal;
	}

	// angleラジアンだけY軸まわりに回転。半径は25
	constexpr float camRange = 25.f;	// targetLength
	camera->rotation(camRange, angle.x, angle.y);


	// 移動量
	const float moveSpeed = 75.f / dxBase->getFPS();
	// 視点移動
	if (input->hitKey(DIK_W)) {
		camera->moveForward(moveSpeed);
	} else if (input->hitKey(DIK_S)) {
		camera->moveForward(-moveSpeed);
	}
	if (input->hitKey(DIK_A)) {
		camera->moveRight(-moveSpeed);
	} else if (input->hitKey(DIK_D)) {
		camera->moveRight(moveSpeed);
	}

	XMFLOAT3 playerPos = camera->getTarget();
	/*playerPos.x -= camRange / 2.f;
	playerPos.y -= camRange / 2.f;*/

	obj3d[0].position = playerPos;

	obj3d[0].rotation.x = XMConvertToDegrees(-angle.x);
	obj3d[0].rotation.y = XMConvertToDegrees(angle.y);
}

void PlayScene::updateLight() {
	// 一秒で一周(2PI[rad])
	const float timeAngle = float(timer->getNowTime()) / Time::oneSec * XM_2PI;

	debugText->formatPrint(spriteBase.get(),
						   WinAPI::window_width / 2.f, DebugText::fontHeight * 16.f, 1.f,
						   XMFLOAT4(1, 1, 0, 1),
						   "light angle : %f PI [rad]\n\t\t\t->%f PI [rad]",
						   timeAngle / XM_PI,
						   angleRoundRad(timeAngle) / XM_PI);

	constexpr float lightR = 20.f;
	lightObj->position = obj3d[0].position;
	lightObj->position.x += nearSin(timeAngle) * lightR;
	lightObj->position.y += nearSin(timeAngle) * lightR;
	lightObj->position.z += nearCos(timeAngle) * lightR;

	light->setLightPos(lightObj->position);
}

void PlayScene::updateSprite() {
	if (input->hitKey(DIK_I)) sprites[0].position.y -= 10; else if (input->hitKey(DIK_K)) sprites[0].position.y += 10;
	if (input->hitKey(DIK_J)) sprites[0].position.x -= 10; else if (input->hitKey(DIK_L)) sprites[0].position.x += 10;

	// Pを押すたびパーティクル50粒追加
	if (input->triggerKey(DIK_P)) {
		constexpr UINT particleNumMax = 50U, particleNumMin = 20U;
		UINT particleNum = particleNumMin;

		float startScale = 5.f;

		if (input->hitKey(DIK_U)) {
			particleNum = particleNumMax;
			startScale = 10.f;
		}
		createParticle(obj3d[0].position, particleNum, startScale);

		Sound::SoundPlayWave(soundBase.get(), particleSE.get());
	}
}

void PlayScene::update_play() {

	// SPACEでENDシーンへ
	if (input->triggerKey(DIK_SPACE)) {
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

	// カメラの更新
	updateCamera();

	// ライトの更新
	updateLight();

	// スプライトの更新
	updateSprite();

#pragma region 情報表示

	if (input->triggerKey(DIK_T)) {
		debugText->tabSize++;
		if (input->hitKey(DIK_LSHIFT)) debugText->tabSize = 4U;
	}


	debugText->formatPrint(spriteBase.get(),
						   DebugText::fontWidth * 2.f, DebugText::fontHeight * 17.f,
						   1.f,
						   XMFLOAT4(1, 1, 1, 1),
						   "newLine\ntab(size %u)\tendString", debugText->tabSize);

#pragma endregion 情報表示
}

#pragma endregion 更新関数

void PlayScene::update() {
	// シーン遷移中も背景は回す
	backObj->rotation.y += 0.1f;

	// 主な処理
	update_proc();

	// 背景オブジェクトの中心をカメラにする
	backObj->position = camera->getEye();

	// ライトとカメラの更新
	light->update();
	camera->update();
}

// シーン開始時の演出用
void PlayScene::update_start() {
	drawAlpha = (float)timer->getNowTime() / sceneTransTime;

	if (drawAlpha > 1.f) {
		drawAlpha = 1.f;

		fbxObj3d->playAnimation();
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
	if (Sound::checkPlaySound(soundData1.get())) {
		Sound::SoundStopWave(soundData1.get());
	}

	for (Sprite &i : sprites) {
		i.isInvisible = true;
	}

	// fbxのアニメーションを停止する
	fbxObj3d->stopAnimation(false);

	drawAlpha = 1.f;
	PostEffect::getInstance()->setAlpha(drawAlpha);

	update_proc = std::bind(&PlayScene::update_end, this);
	timer->reset();
}

void PlayScene::drawObj3d() {

	Object3d::startDraw(dxBase->getCmdList(), backPipelineSet);
	backObj->drawWithUpdate(dxBase, light.get());

	ParticleMgr::startDraw(dxBase->getCmdList(), object3dPipelineSet);
	particleMgr->drawWithUpdate(dxBase->getCmdList());

	Object3d::startDraw(dxBase->getCmdList(), object3dPipelineSet);
	lightObj->drawWithUpdate(dxBase, light.get());
	for (UINT i = 0; i < obj3d.size(); ++i) {
		obj3d[i].drawWithUpdate(dxBase, light.get());
	}

	fbxObj3d->drawWithUpdate(dxBase->getCmdList(), light.get());
}

void PlayScene::drawFrontSprite() {
	drawImGui();

	spriteBase->drawStart(dxBase->getCmdList());
	// スプライト描画
	for (UINT i = 0, len = (UINT)sprites.size(); i < len; ++i) {
		sprites[i].drawWithUpdate(dxBase, spriteBase.get());
	}

	// デバッグテキスト描画
	debugText->DrawAll(dxBase, spriteBase.get());
}

void PlayScene::drawImGui() {
	constexpr auto winFlags =
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

	if (guiWinAlive) {
		ImGui::Begin(u8"情報表示", &guiWinAlive, winFlags);
		//ImGui::SetWindowPos(ImVec2(20, 20));
		//ImGui::SetWindowSize(ImVec2(300, 300));
		ImGui::Text("x = %.1f, y = %.1f", ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
		ImGui::Text("FPS <- %.3f", dxBase->getFPS());
		ImGui::Text(u8"時間 <- %.6f秒",
					float(timer->getNowTime()) / float(Time::oneSec));
		ImGui::Text(u8"BGM再生状態 <- %s",
					Sound::checkPlaySound(soundData1.get())
					? u8"再生|>"
					: u8"停止[]");
		// 次のウインドウは今のウインドウのすぐ下
		ImGui::SetNextWindowPos(getWindowLBPos());
		ImGui::End();
	}

	ImGui::Begin(u8"操作説明", nullptr, winFlags);
	//ImGui::SetWindowPos(ImVec2(20, 200));

	ImGui::Text(u8"0 : BGM再生/停止");
	ImGui::Text(u8"SPACE : 終了");
	ImGui::Text(u8"WASD : カメラ移動");
	ImGui::Text(u8"arrow : カメラ回転");
	ImGui::Text(u8"P : ﾊﾟｰﾃｨｸﾙ生成(SE再生)");
	ImGui::Text(u8"M : シェーダー変更");
	ImGui::SetNextWindowPos(getWindowLBPos());
	ImGui::End();

	ImGui::Begin(u8"てすと", nullptr, winFlags);
	//ImGui::SetWindowPos(ImVec2(20, 400));
	ImGui::SetWindowSize(ImVec2(200, 300));
	ImGui::Separator();
	if (ImGui::CollapsingHeader(u8"にゅうりょくへっだぁ")) {
		static bool selected = true;
		ImGui::Selectable(u8"選べる文字だよ！", &selected);
		static bool checked = true;
		ImGui::Checkbox(u8"ちぇっくぼっくす", &checked);
		ImGui::Separator();
		ImGui::Text(u8"ラジオボタン");
		static int radioBotton = 0;
		ImGui::RadioButton(u8"ラジオ0", &radioBotton, 0);
		ImGui::RadioButton(u8"ラジオ1", &radioBotton, 1);
		ImGui::RadioButton(u8"ラジオ2", &radioBotton, 2);
		ImGui::Separator();
		static float fNum = 0.f;
		ImGui::DragFloat(u8"どらっぐ", &fNum, 0.001f, -0.1f, 0.1f);
		static float fNum2 = 0.f;
		ImGui::SliderFloat(u8"すらいだぁ", &fNum2,
						   -0.1f, 0.1f, "%.5f",
						   ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
	}
	if (ImGui::CollapsingHeader(u8"そのたのへっだぁ")) {
		ImGui::TextColored(ImVec4(1.f, 0.5f, 1.f, 1.f), u8"マゼンタ");
	}
	ImGui::End();
}

void PlayScene::createParticle(const DirectX::XMFLOAT3 &pos,
							   const UINT particleNum,
							   const float startScale) {
	for (UINT i = 0U; i < particleNum; ++i) {

		const float theata = RandomNum::getRandf(0, XM_PI);
		const float phi = RandomNum::getRandf(0, XM_PI * 2.f);
		const float r = RandomNum::getRandf(0, 5.f);

		XMFLOAT3 generatePos = pos;

		const XMFLOAT3 vel{
			r * nearSin(theata) * nearCos(phi),
			r * nearCos(theata),
			r * nearSin(theata) * nearSin(phi)
		};

		XMFLOAT3 acc{};


		constexpr XMFLOAT3 startCol = XMFLOAT3(1, 1, 0.25f), endCol = XMFLOAT3(1, 0, 1);
		constexpr int life = Time::oneSec / 4;
		constexpr float endScale = 0.f;
		constexpr float startRota = 0.f, endRota = 0.f;

		// 追加
		particleMgr->add(timer.get(),
						 life, generatePos, vel, acc,
						 startScale, endScale,
						 startRota, endRota,
						 startCol, endCol);
	}
}

PlayScene::~PlayScene() {
	//Sound::SoundStopWave(soundData1.get());
}