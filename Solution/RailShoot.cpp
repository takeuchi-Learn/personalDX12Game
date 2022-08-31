#include "RailShoot.h"
#include <DirectXMath.h>

#include "SceneManager.h"
#include "PlayScene.h"

#include "PostEffect.h"

#include "Collision.h"

#include "RandomNum.h"

#include <fstream>

using namespace DirectX;

const Time::timeType RailShoot::sceneChangeTime = Time::oneSec;

// std::stringの2次元配列(vector)
using CSVType = std::vector<std::vector<std::string>>;
// @brief loadCsvの入力をstd::stringにしたもの
// @return 読み込んだcsvの中身。失敗したらデフォルトコンストラクタで初期化された空のvector2次元配列が返る
// @param commentFlag //で始まる行を無視するかどうか(trueで無視)
// @param divChar フィールドの区切り文字
// @param commentStartStr コメント開始文字
RailShoot::CSVType RailShoot::loadCsv(const std::string &csvFilePath,
									  bool commentFlag,
									  char divChar,
									  const std::string &commentStartStr) {
	CSVType csvData{};	// csvの中身を格納

	std::ifstream ifs(csvFilePath);
	if (!ifs) {
		return csvData;
	}

	std::string line{};
	// 開いたファイルを一行読み込む(カーソルも動く)
	while (std::getline(ifs, line)) {
		// コメントが有効かつ行頭が//なら、その行は無視する
		if (commentFlag && line.find(commentStartStr) == 0U) {
			continue;
		}

		// 行数を増やす
		csvData.emplace_back();

		std::istringstream stream(line);
		std::string field;
		// 読み込んだ行を','区切りで分割
		while (std::getline(stream, field, divChar)) {
			csvData.back().emplace_back(field);
		}
	}

	return csvData;
}

XMVECTOR RailShoot::splinePosition(const std::vector<XMVECTOR> &points,
								   size_t startIndex,
								   float t) {
	if (startIndex < 1) return points[1];

	{
		size_t n = points.size() - 2;
		if (startIndex > n)return points[n];
	}

	XMVECTOR p0 = points[startIndex - 1];
	XMVECTOR p1 = points[startIndex];
	XMVECTOR p2 = points[startIndex + 1];
	XMVECTOR p3 = points[startIndex + 2];

	XMVECTOR position = {
		2 * p1 + (-p0 + p2) * t +
		(2 * p0 - 5 * p1 + 4 * p2 - p3) * (t * t) +
		(-p0 + 3 * p1 - 3 * p2 + p3) * (t * t * t)
	};
	position *= 0.5f;

	return position;
}

namespace {
	std::vector<XMVECTOR> splinePoint;
	UINT splineNowFrame = 0u;
	constexpr UINT splineFrameMax = 120u;
	constexpr UINT splineIndexDef = 1u;
	UINT splineIndex = splineIndexDef;
}

RailShoot::RailShoot()
	: dxBase(DX12Base::getInstance()),
	input(Input::getInstance()),

	update_proc(std::bind(&RailShoot::update_start, this)),

	camera(std::make_unique<CameraObj>(nullptr)),
	light(std::make_unique<Light>()),

	timer(std::make_unique<Time>()),

	spriteBase(std::make_unique<SpriteBase>()),

	back(std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true)),
	enemyModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),
	enemyBulModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),
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

	const UINT aimPosTexNum = spriteBase->loadTexture(L"Resources/aimPos.png");
	aim2D = std::make_unique<Sprite>(aimPosTexNum, spriteBase.get());

	// カメラ初期化
	camera->setFarZ(5000.f);
	camera->setEye(XMFLOAT3(0, WinAPI::getInstance()->getWindowSize().y * 0.06f, -180));	// 視点座標
	camera->setTarget(XMFLOAT3(0, 0, 0));	// 注視点座標
	camera->setUp(XMFLOAT3(0, 1, 0));		// 上方向
	//camera->update();

	// ライト初期化
	light->setLightPos(camera->getEye());

	// 自機初期化
	constexpr XMFLOAT3 playerStartPos = XMFLOAT3(0, 0, 0);
	player = std::make_unique<Player>(camera.get(), playerModel.get(), playerStartPos);
	player->setScale(10.f);

	camera->setParentObj(player.get());
	camera->update();

	// スプライン
	// startとendは2つ必要
	splinePoint.emplace_back(XMVectorSet(0, 0, 0, 1));		// start
	splinePoint.emplace_back(XMVectorSet(0, 0, 0, 1));		// start
	splinePoint.emplace_back(XMVectorSet(0, 0, 200, 1));	// 経由1
	splinePoint.emplace_back(XMVectorSet(20, 50, 400, 1));	// 経由2
	splinePoint.emplace_back(XMVectorSet(20, 50, 600, 1));	// 経由3
	splinePoint.emplace_back(XMVectorSet(0, 100, 800, 1));	// end
	splinePoint.emplace_back(XMVectorSet(0, 100, 800, 1));	// end

	// 敵初期化
	enemy.resize(0U);

	// 天球
	const float backScale = camera->getFarZ() * 0.9f;
	back->setScale({ backScale, backScale, backScale });

	// 敵発生スクリプト
	csvData = loadCsv("Resources/enemyScript.csv", true, ',', "//");
	{
		UINT waitFrame = 0u;
		for (auto &y : csvData) {
			if (y[0] == "WAIT") {
				waitFrame += (UINT)std::stoi(y[1]);
			} else if (y[0] == "PUSH") {
				enemyPopData.emplace_front(std::make_unique<PopEnemyData>(waitFrame,
																		  XMFLOAT3(std::stof(y[1]),
																				   std::stof(y[2]),
																				   std::stof(y[3])),
																		  XMFLOAT3(0, 0, -1)));
			}
		}
	}
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

void RailShoot::addEnemy(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &vel, float scale) {
	auto &i = enemy.emplace_front(new Enemy(camera.get(), enemyModel.get(), enemyBulModel.get(), pos));
	i->setScale(scale);
	i->setVel(vel);
	i->setTargetObj(player.get());
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
	if (input->hitKey(DIK_LSHIFT) && input->hitKey(DIK_SPACE)) {
		changeNextScene();
	}

	// 敵を増やす
	// 終わった発生情報は削除
	enemyPopData.remove_if([&](std::unique_ptr<PopEnemyData> &i) {
		const bool ended = nowFrame >= i->popFrame;
		if (ended) {
			addEnemy(i->pos, i->vel);
		}
		return ended; });

	const bool hitW = input->hitKey(DIK_W);
	const bool hitA = input->hitKey(DIK_A);
	const bool hitS = input->hitKey(DIK_S);
	const bool hitD = input->hitKey(DIK_D);

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
	const bool hitUp = input->hitKey(DIK_UP);
	const bool hitDown = input->hitKey(DIK_DOWN);
	const bool hitRight = input->hitKey(DIK_RIGHT);
	const bool hitLeft = input->hitKey(DIK_LEFT);

	if (hitUp || hitDown || hitRight || hitLeft) {
		const bool speed = 90.f / dxBase->getFPS();

		XMFLOAT3 rota = player->getRotation();

		if (hitUp) {
			rota.x -= speed;
			if (rota.x <= -360.f) {
				rota.x += 360.f;
			}
		} else if (hitDown) {
			rota.x += speed;
			if (rota.x >= 360.f) {
				rota.x -= 360.f;
			}
		}

		if (hitRight) {
			rota.y += speed;
			if (rota.y >= 180.f) {
				rota.y -= 360.f;
			}
		} else if (hitLeft) {
			rota.y -= speed;
			if (rota.y <= -180.f) {
				rota.y += 360.f;
			}
		}

		player->setRotation(rota);

		debugText->formatPrint(spriteBase.get(), 0, 500, 1.f, { 1,0,1,1 },
							   "%.3f, %.3f", rota.x, rota.y);
	}

	// Z座標が0を超えたら退場
	for (auto &i : enemy) {
		if (i->getPos().z < 0.f) {
			i->chansePhase_Leave(DirectX::XMFLOAT3(-1, 1, 0));
		}
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

			pBulCol = Sphere(XMLoadFloat3(&pb.getPos()), pb.getScale());

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

		// 自機と敵弾の当たり判定
		if (player->getAlive()) {
			const Sphere playerCol(XMLoadFloat3(&player->getPos()), player->getScale());

			for (auto &e : enemy) {
				for (auto &eb : e->getBulList()) {
					//　存在しない敵弾の処理はしない
					if (!eb->getAlive()) continue;

					// 自機と敵の弾が当たっていたら
					if (Collision::CheckHit(playerCol,
											Sphere(XMLoadFloat3(&eb->getPos()),
												   eb->getScaleF3().z))) {
						// 次のシーンへ進む(仮)
						//changeNextScene();
					}
				}
			}
		}

		// 弾がなく、かつ死んだ敵は消す
		enemy.remove_if([](const std::unique_ptr<Enemy> &i) {return !i->getAlive() && i->bulEmpty(); });

		// 敵がすべて消えたら次のシーンへ
		if (enemy.empty()) {
			changeNextScene();
		}
	}

	light->setLightPos(camera->getEye());

	++nowFrame;
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
	constexpr ImGuiWindowFlags winFlags =
		// リサイズ不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		// タイトルバー無し
		//ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
		// 設定を.iniに出力しない
		ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings |
		// 移動不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
		// スクロールバーを常に表示
		ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysHorizontalScrollbar |
		ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar;

	// 最初のウインドウの位置を指定
	ImGui::SetNextWindowPos(ImVec2((float)WinAPI::window_width * 0.02f,
								   (float)WinAPI::window_height * 0.02f));

	ImGui::Begin("レールシューティング", nullptr, winFlags);
	ImGui::Text("スクリプトで指定したとおりに敵が出る");
	ImGui::Text("敵は一定間隔で弾を発射する");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::End();

	ImGui::Begin("情報", nullptr, winFlags);
	ImGui::Text("FPS : %.3f", dxBase->getFPS());
	ImGui::Text("敵数 : %u", std::distance(enemy.begin(), enemy.end()));
	ImGui::Text("経過フレーム : %u", nowFrame);
	ImGui::End();

	spriteBase->drawStart(dxBase->getCmdList());
	aim2D->position = XMFLOAT3(player->getAim2DPos().x, player->getAim2DPos().y, 0.f);
	aim2D->drawWithUpdate(dxBase, spriteBase.get());
	debugText->DrawAll(dxBase, spriteBase.get());
}

RailShoot::~RailShoot() {
	PostEffect::getInstance()->setAlpha(1.f);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::window_width,
													 WinAPI::window_height));
	PostEffect::getInstance()->changePipeLine(0U);
}