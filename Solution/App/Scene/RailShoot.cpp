#include "RailShoot.h"
#include <DirectXMath.h>

#include "BossScene.h"
#include "GameOverScene.h"

#include <fstream>
#include "Util/RandomNum.h"
#include "System/PostEffect.h"
#include "Collision/Collision.h"
#include "System/SceneManager.h"

#include <3D/Fbx/FbxLoader.h>

#ifdef min
#undef min
#endif // min

#ifdef max
#undef max
#endif // max

using namespace DirectX;

namespace
{
	inline XMFLOAT3 operator+(const XMFLOAT3& r, const XMFLOAT3& l)
	{
		return XMFLOAT3(r.x + l.x,
						r.y + l.y,
						r.z + l.z);
	}
	inline XMFLOAT3 operator-(const XMFLOAT3& r, const XMFLOAT3& l)
	{
		return XMFLOAT3(r.x - l.x,
						r.y - l.y,
						r.z - l.z);
	}

	void operator+=(XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		lhs.x += rhs.x;
		lhs.y += rhs.y;
		lhs.z += rhs.z;
	}

	void operator-=(XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		lhs.x -= rhs.x;
		lhs.y -= rhs.y;
		lhs.z -= rhs.z;
	}

	constexpr XMFLOAT3 lerp(const XMFLOAT3& r, const XMFLOAT3& l, float t)
	{
		return XMFLOAT3(std::lerp(r.x, l.x, t),
						std::lerp(r.y, l.y, t),
						std::lerp(r.z, l.z, t));
	}
}

RailShoot::CSVType RailShoot::loadCsv(const std::string& csvFilePath,
									  bool commentFlag,
									  char divChar,
									  const std::string& commentStartStr)
{
	CSVType csvData{};	// csvの中身を格納

	std::ifstream ifs(csvFilePath);
	if (!ifs)
	{
		return csvData;
	}

	std::string line{};
	// 開いたファイルを一行読み込む(カーソルも動く)
	while (std::getline(ifs, line))
	{
		// コメントが有効かつ行頭が//なら、その行は無視する
		if (commentFlag && line.find(commentStartStr) == 0U)
		{
			continue;
		}

		// 行数を増やす
		csvData.emplace_back();

		std::istringstream stream(line);
		std::string field;
		// 読み込んだ行を','区切りで分割
		while (std::getline(stream, field, divChar))
		{
			csvData.back().emplace_back(field);
		}
	}

	return csvData;
}

void RailShoot::loadEnemyScript()
{
	csvData = loadCsv("Resources/enemyScript.csv", true, ',', "//");
	{
		for (auto& y : csvData)
		{
			enemyPopData.emplace_front(
				std::make_unique<PopEnemyData>((uint16_t)std::stoul(y[3]),
											   XMFLOAT3(std::stof(y[0]),
														std::stof(y[1]),
														std::stof(y[2])),
											   XMFLOAT3(0, 0, -1)));
		}
	}
}

XMVECTOR RailShoot::splinePosition(const std::vector<XMVECTOR>& points,
								   const size_t& startIndex,
								   float t)
{
	if (startIndex < 1) { return points[1]; }

	{
		size_t n = points.size() - 2;
		if (startIndex > n) { return points[n]; }
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

RailShoot::RailShoot()
	: dxBase(DX12Base::getInstance()),
	input(Input::getInstance()),

	// --------------------
	// 更新関数の格納変数
	// --------------------
	update_proc(std::bind(&RailShoot::update_start, this)),

	// --------------------
	// シーンに必要な諸要素
	// --------------------
	camera(std::make_unique<CameraObj>(nullptr)),
	light(std::make_unique<Light>()),

	timer(std::make_unique<Timer>()),

	spriteBase(std::make_unique<SpriteBase>(SpriteBase::BLEND_MODE::ALPHA)),

	// --------------------
	// 敵モデル
	// --------------------
	enemyModel(std::make_unique<ObjModel>("Resources/tori", "tori", 0U, true)),
	enemyBulModel(std::make_unique<ObjModel>("Resources/bullet", "bullet", 0U, true)),

	// --------------------
	// 自機関連
	// --------------------
	playerModel(std::make_unique<ObjModel>("Resources/player", "player")),
	playerBulModel(std::make_unique<ObjModel>("Resources/bullet", "bullet", 0U, true)),
	playerHpMax(20u),

	// --------------------
	// レール現在位置のオブジェクト
	// --------------------
	railObj(std::make_unique<GameObj>(camera.get())),

	// --------------------
	// パーティクル
	// --------------------
	particleMgr(std::make_unique<ParticleMgr>(L"Resources/effect1.png", camera.get())),

	startSceneChangeTime(0U),

	backPipelineSet(Object3d::createGraphicsPipeline(Object3d::BLEND_MODE::ALPHA,
													 L"Resources/Shaders/BackVS.hlsl",
													 L"Resources/Shaders/BackPS.hlsl")),

	// --------------------
	// スプライト初期化
	// --------------------
	debugText(new DebugText(spriteBase->loadTexture(L"Resources/debugfont.png"),
							spriteBase.get())),

	aim2D(new Sprite(spriteBase->loadTexture(L"Resources/aimPos.png"),
					 spriteBase.get())),

	hpBar(new Sprite(spriteBase->loadTexture(L"Resources/hpBar.png"),
					 spriteBase.get(),
					 XMFLOAT2(0.f, 1.f))),
	hpBarEdge(new Sprite(spriteBase->loadTexture(L"Resources/hpBar.png"),
						 spriteBase.get(),
						 XMFLOAT2(0.f, 1.f))),
	hpBarWidMax(WinAPI::window_width * 0.25f),
	operInstPosR(WinAPI::window_width * 0.1f)
{
	hpBar->color = XMFLOAT4(0, 0.5f, 1, 1);
	hpBar->position = XMFLOAT3(WinAPI::window_width / 20.f, WinAPI::window_height, 0.f);
	{
		XMFLOAT2 size = hpBar->getSize();
		size.x = hpBarWidMax;
		size.y = (float)WinAPI::window_height / 32.f;
		hpBar->setSize(size);

		hpBar->position.y -= size.y;
	}
	hpBarEdge->position = hpBar->position;
	hpBarEdge->setSize(hpBar->getSize());

	// 操作説明
	constexpr XMFLOAT3 centerPos = XMFLOAT3(WinAPI::window_width / 2.f, WinAPI::window_height * 0.75f, 0.f);

	operInst["W"] = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/OperInst/W.png"),
											 spriteBase.get(),
											 XMFLOAT2(0.5f, 0.5f));
	operInst["W"]->position.x = centerPos.x;
	operInst["W"]->position.y = centerPos.y - operInstPosR;

	operInst["S"] = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/OperInst/S.png"),
											 spriteBase.get(),
											 XMFLOAT2(0.5f, 0.5f));
	operInst["S"]->position.x = centerPos.x;
	operInst["S"]->position.y = centerPos.y + operInstPosR;

	operInst["A"] = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/OperInst/A.png"),
											 spriteBase.get(),
											 XMFLOAT2(0.5f, 0.5f));
	operInst["A"]->position.x = centerPos.x - operInstPosR;
	operInst["A"]->position.y = centerPos.y;

	operInst["D"] = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/OperInst/D.png"),
											 spriteBase.get(),
											 XMFLOAT2(0.5f, 0.5f));
	operInst["D"]->position.x = centerPos.x + operInstPosR;
	operInst["D"]->position.y = centerPos.y;

	operInst["Mouse_L"] = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/OperInst/Mouse_L.png"),
												   spriteBase.get(),
												   XMFLOAT2(0.f, 0.f));

	for (auto& i : operInst)
	{
		i.second->update(spriteBase.get());
		i.second->color = XMFLOAT4(1, 1, 1, 0.5f);
		i.second->isInvisible = true;
	}

#pragma region 音

	killSe = std::make_unique<Sound>("Resources/SE/Sys_Set03-click.wav");
	bgm = std::make_unique<Sound>("Resources/BGM/A-Sense-of-Loss.wav");

#pragma endregion 音

	// --------------------
	// カメラ初期化
	// --------------------
	camera->setFarZ(5000.f);
	camera->setEye(XMFLOAT3(0, WinAPI::getInstance()->getWindowSize().y * 0.06f, -180.f));	// 視点座標
	camera->setTarget(XMFLOAT3(0, 0, 0));	// 注視点座標
	camera->setUp(XMFLOAT3(0, 1, 0));		// 上方向
	camera->setFogAngleYRad(camFogEnd);		// フォグ
	camera->setEye2TargetLen(200.f);

	// --------------------
	// ライト初期化
	// --------------------
	light->setLightPos(camera->getEye());

	// --------------------
	// 自機初期化
	// --------------------
	player = std::make_unique<Player>(camera.get(), playerModel.get());
	player->setScale(16.f);
	player->setParent(railObj.get());
	player->setPos(XMFLOAT3(0, 12.f, 0));
	player->setHp(playerHpMax);

	// --------------------
	// 背景と地面
	// --------------------

	// 背景の天球
	{
		backModel.reset(new ObjModel("Resources/back/", "back", 0U, true));
		backObj.reset(new Object3d(camera.get(), backModel.get()));
		const float backScale = camera->getFarZ() * 0.9f;
		backObj->scale = XMFLOAT3(backScale, backScale, backScale);
	}

	// 地面
	{
		groundModel.reset(new ObjModel("Resources/ground", "ground"));
		groundObj.reset(new Object3d(camera.get(), groundModel.get()));

		constexpr UINT groundSize = 5000u;
		groundObj->position = XMFLOAT3(0, -player->getScale() * 3.f, (float)groundSize);

		groundObj->scale = XMFLOAT3(groundSize, groundSize, groundSize);

		constexpr float tillingNum = groundSize / 32.f;
		groundModel->setTexTilling(XMFLOAT2(tillingNum, tillingNum));
	}

	// --------------------
	// スプライン
	// --------------------

	// レールの情報読み込み
	/// todo 関数化
	{
		// 制御点の情報はCSVから読み込む
		csvData = loadCsv("Resources/splinePos.csv", true, ',', "//");

		// 始点は原点
		// startは2つ必要
		splinePoint.emplace_back(XMVectorSet(0, 0, 0, 1));
		splinePoint.emplace_back(XMVectorSet(0, 0, 0, 1));

		// CSVの内容を配列に格納
		for (auto& y : csvData)
		{
			splinePoint.emplace_back(XMVectorSet(std::stof(y[0]),
												 std::stof(y[1]),
												 std::stof(y[2]),
												 1));
		}

		// endも2つ必要
		splinePoint.emplace_back(splinePoint.back());
	}
	{
		// モデルを読み込む
		constexpr UINT wallModelTexNum = 0u;
		wallModel.reset(new ObjModel("Resources/laneWall", "laneWall", wallModelTexNum, false));
		ringModel.reset(new ObjModel("Resources/ring", "ring", wallModelTexNum, false));

		// 制御点の数だけオブジェクトを置く
		const size_t splinePointNum = splinePoint.size() - 2u;
		const size_t wallNum = splinePointNum;
		laneWall.resize(wallNum);

		XMFLOAT3 dest{};
		XMFLOAT3 preLanePos{};
		for (UINT y = 0u; y < wallNum; ++y)
		{
			laneWall[y].resize(3);

			auto& right = laneWall[y][0] = std::make_unique<Object3d>(camera.get(), wallModel.get());
			auto& left = laneWall[y][1] = std::make_unique<Object3d>(camera.get(), wallModel.get());
			auto& ring = laneWall[y][2] = std::make_unique<Object3d>(camera.get(), ringModel.get());

			// --------------------
			// オブジェクトの大きさを変更
			// --------------------
			constexpr float scale = 96.f;
			constexpr XMFLOAT3 scalef3(scale, scale, scale);
			right->scale = XMFLOAT3(16, 16 * 16, 16);
			left->scale = XMFLOAT3(16, 16 * 16, 16);
			ring->scale = XMFLOAT3(96, 96, 96);

			// --------------------
			// レーンの位置にする
			// --------------------

			// 全体の割合
			const float allRaito = (float)y / (float)wallNum;

			// 全体の割合(0 ~ 制御点の数)
			float startIndexRaito = allRaito * (float)splinePointNum;

			// 制御点間の割合とインデックスを取得
			float startIndex = 0.f;
			startIndexRaito = std::modf(startIndexRaito, &startIndex);

			// 前の点の位置を取っておく
			if (y > 0u)
			{
				preLanePos = dest;
			}

			// スプライン補間でレーンの位置を求める
			XMStoreFloat3(&dest, splinePosition(splinePoint, (size_t)startIndex, startIndexRaito));
			right->position = dest;
			left->position = dest;
			ring->position = dest;

			right->position.y = groundObj->position.y + 16 * 16;
			left->position.y = groundObj->position.y + 16 * 16;

			// --------------------
			// レーンの左右に配置する
			// --------------------
			constexpr float laneR = 192.f;

			// 右方向を取得
			XMFLOAT2 velRota = GameObj::calcRotationSyncVelRad(dest - preLanePos);
			const XMVECTOR rightVec = XMVector3Rotate(XMVectorSet(laneR, 0, 0, 0),
													  XMQuaternionRotationRollPitchYaw(velRota.x,
																					   velRota.y,
																					   0.f));
			// XMFLOAT3にする
			XMFLOAT3 rightPos{};
			XMStoreFloat3(&rightPos, rightVec);

			//// 左右にずらす
			right->position += rightPos;
			left->position -= rightPos;

			// 色を変更
			constexpr float colVal = 0.5f;
			constexpr XMFLOAT4 col = XMFLOAT4(colVal, colVal, colVal, 1.f);
			right->color = col;
			left->color = col;
			ring->color = col;

			// 度数法に変換
			velRota.x = XMConvertToDegrees(velRota.x);
			velRota.y = XMConvertToDegrees(velRota.y);
			// 回転を反映
			ring->rotation = XMFLOAT3(velRota.x, velRota.y, 0);
		}
	}

	// --------------------
	// 敵初期化
	// --------------------

	// 敵は最初居ない
	enemy.resize(0U);

	// 敵発生スクリプト
	loadEnemyScript();
}

void RailShoot::start()
{
	aim2D->isInvisible = true;
	for (auto& i : operInst)
	{
		i.second->isInvisible = true;
	}

	// カメラは固定カメラ
	initFixedCam(appearPPosStart, appearPPosEnd);

	// マウスカーソルは表示しない
	input->changeDispMouseCursorFlag(false);

	// bgm鳴らす
	Sound::SoundPlayWave(bgm.get(), XAUDIO2_LOOP_INFINITE, 0.2f);

	// タイマー開始
	timer->reset();
	startSceneChangeTime = timer->getNowTime();
}

void RailShoot::update()
{
#ifdef _DEBUG

	if (input->hitKey(DIK_LSHIFT) && input->triggerKey(DIK_SPACE))
	{
		changeNextScene<BossScene>();
	}

#endif // _DEBUG

	rotationBack();

	// 背景オブジェクトの中心をカメラにする
	backObj->position = camera->getEye();

	// 主な処理
	update_proc();

	// レールの現在位置オブジェクト更新
	railObj->update();

	// ライトとカメラの更新
	light->update();
	camera->update();
}

void RailShoot::createParticle(const DirectX::XMFLOAT3& pos,
							   const uint16_t particleNum,
							   const float startScale,
							   const float vel)
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

		constexpr XMFLOAT3 startCol = XMFLOAT3(1, 1, 0.25f), endCol = XMFLOAT3(1, 0, 1);
		constexpr Timer::timeType life = Timer::oneSec / 4;
		constexpr float endScale = 0.f;
		constexpr float startRota = 0.f, endRota = 0.f;

		// 追加
		particleMgr->add(life, pos, vel, acc,
						 startScale, endScale,
						 startRota, endRota,
						 startCol, endCol);
	}
}

void RailShoot::startRgbShift()
{
	rgbShiftFlag = true;
	startRgbShiftTime = timer->getNowTime();
}

void RailShoot::updateRgbShift()
{
	if (!rgbShiftFlag) { return; }

	const auto nowRgbShiftTime = timer->getNowTime() - startRgbShiftTime;

	const float raito = (float)nowRgbShiftTime / (float)rgbShiftTimeMax;
	if (raito > 1.f)
	{
		PostEffect::getInstance()->setRgbShiftNum({ 0.f, 0.f });
		rgbShiftFlag = false;
		return;
	}

	constexpr float rgbShiftMumMax = 1.f / 16.f;

	constexpr float  c4 = 2.f * XM_PI / 3.f;
	const float easeRate = -std::pow(2.f, 10.f * (1.f - raito) - 10.f) *
		dxBase->nearSin((raito * 10.f - 10.75f) * c4);

	PostEffect::getInstance()->setRgbShiftNum({ easeRate * rgbShiftMumMax, 0.f });
}

void RailShoot::rotationBack()
{
	// シーン遷移中も背景は回す
	XMFLOAT2 shiftUv = backModel->getShiftUv();
	constexpr float shiftSpeed = 0.01f;

	shiftUv.x += shiftSpeed / DX12Base::getInstance()->getFPS();

	backModel->setShivtUv(shiftUv);
}

void RailShoot::addEnemy(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& vel, float scale)
{
	auto& i = enemy.emplace_front(std::make_shared<NormalEnemy>(camera.get(), enemyModel.get(), enemyBulModel.get(), pos));
	i->setScale(scale);
	i->setVel(vel);
	i->setTargetObj(player.get());
	i->setParent(railObj->getObj());
	i->setCol(XMFLOAT4(1, 0.25f, 0.125f, 1.f));
}

template<class NextScene>
void RailShoot::changeNextScene()
{
	PostEffect::getInstance()->changePipeLine(0U);

	update_proc = std::bind(&RailShoot::update_end<NextScene>, this);
	startSceneChangeTime = timer->getNowTime();
}

void RailShoot::update_start()
{
	const Timer::timeType nowTime = timer->getNowTime() - startSceneChangeTime;
	if (nowTime >= sceneChangeTime)
	{
		startAppearPlayer();
	}

	const float timeRaito = (float)nowTime / sceneChangeTime;
	PostEffect::getInstance()->setAlpha(timeRaito);

	const float mosCoe = std::pow(timeRaito, 5.f);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::getInstance()->getWindowSize().x * mosCoe,
													 WinAPI::getInstance()->getWindowSize().y * mosCoe));

	PostEffect::getInstance()->setNoiseIntensity(1.f - timeRaito);

	const float vignNum = 0.5f * timeRaito;
	PostEffect::getInstance()->setVignIntensity(vignNum);

	const float speedLineIntensity = 0.125f * timeRaito;
	PostEffect::getInstance()->setSpeedLineIntensity(speedLineIntensity);
}

void RailShoot::update_appearPlayer()
{
#ifdef _DEBUG
	if (input->triggerKey(DIK_SPACE))
	{
		endAppearPlayer();

		return;
	}
#endif // _DEBUG

	const auto nowTime = appearPlayer->timer->getNowTime();

	if (nowTime > appearPlayer->appearTime)
	{
		endAppearPlayer();

		return;
	}

	const float raito = (float)nowTime / (float)appearPlayer->appearTime;

	const XMFLOAT3 nowPos = lerp(appearPlayer->playerPos.start,
								 appearPlayer->playerPos.end,
								 raito);

	player->setPos(nowPos);

	camera->setTarget(player->calcWorldPos());

	player->setScaleF3(lerp(appearPlayer->playerScale.start,
							appearPlayer->playerScale.end,
							raito));

	// 1->0->1と進む
	const float fogRaito = 2.f * (std::max(raito, 0.5f) - std::min(raito, 0.5f));

	// フォグを変更
	camera->setFogAngleYRad(std::lerp(appearPlayer->camFogRad.start,
									  appearPlayer->camFogRad.end,
									  std::pow(fogRaito, 0.5f)));
}

void RailShoot::update_play()
{
	{
		// マウスカーソルの位置をパッド入力に合わせてずらす
		POINT pos = input->getMousePos();

		XMFLOAT2 rStick = input->getPadRStickRaito();
		float speed = 10.f;
		if (input->getPadButton(Input::PAD::RIGHT_THUMB))
		{
			speed /= 2.f;
		}

		pos.x += static_cast<LONG>(rStick.x * speed);
		pos.y += static_cast<LONG>(-rStick.y * speed);

		input->setMousePos(pos.x, pos.y);
	}

	// 照準の位置をマウスカーソルに合わせる
	player->setAim2DPos(XMFLOAT2((float)input->getMousePos().x,
								 (float)input->getMousePos().y));
	// 照準の位置に照準画像を表示
	aim2D->position = XMFLOAT3(player->getAim2DPos().x, player->getAim2DPos().y, 0.f);

	// 操作説明の位置
	{
		if (!operInst.at("Mouse_L")->isInvisible)
		{
			operInst.at("Mouse_L")->position = XMFLOAT3(aim2D->position.x,
														aim2D->position.y,
														0);
		}

		const bool dispW = !operInst.at("W")->isInvisible;
		const bool dispS = !operInst.at("S")->isInvisible;
		const bool dispA = !operInst.at("A")->isInvisible;
		const bool dispD = !operInst.at("D")->isInvisible;

		if (dispW || dispS || dispA || dispD)
		{
			const XMFLOAT2 pposF2 = player->getObj()->calcScreenPos();
			const XMFLOAT3 playerPos2D = XMFLOAT3(pposF2.x, pposF2.y, 0.f);

			if (dispW)
			{
				operInst.at("W")->position = playerPos2D;
				operInst.at("W")->position.y -= operInstPosR;
			}
			if (dispS)
			{
				operInst.at("S")->position = playerPos2D;
				operInst.at("S")->position.y += operInstPosR;
			}
			if (dispA)
			{
				operInst.at("A")->position = playerPos2D;
				operInst.at("A")->position.x -= operInstPosR;
			}
			if (dispD)
			{
				operInst.at("D")->position = playerPos2D;
				operInst.at("D")->position.x += operInstPosR;
			}
		}
	}

	// --------------------
	// 敵を増やす
	// --------------------

	// 終わった発生情報は消費して削除
	enemyPopData.remove_if([&](std::unique_ptr<PopEnemyData>& i)
						   {
							   const bool ended = nowFrame >= i->popFrame;
	if (ended)
	{
		addEnemy(i->pos, i->vel);
	}
	return ended;
						   });

	// --------------------
	// レール現在位置オブジェクト
	// --------------------
	updateRailPos();

	// 自機移動回転
	movePlayer();

	// --------------------
	// 弾発射
	// --------------------
	updateAimCol();
	if (input->triggerMouseButton(Input::MOUSE::LEFT) ||
		input->triggerPadButton(Input::PAD::RB) ||
		input->triggerPadButton(Input::PAD::A) ||
		input->triggerPadButton(Input::PAD::B))
	{
		updatePlayerShotTarget();
		if (!player->getShotTarget().expired())
		{
			constexpr float bulSpeed = 2.f;
			player->shot(camera.get(), playerBulModel.get(), bulSpeed);

			operInst.at("Mouse_L")->isInvisible = true;
		}
	}

	// --------------------
	// 当たり判定
	// --------------------
	{
		// --------------------
		// 自機弾と敵の当たり判定
		// --------------------
		Sphere pBulCol{};
		for (auto& pb : player->getBulArr())
		{
			if (!pb.getAlive()) { continue; }

			pBulCol = Sphere(XMLoadFloat3(&pb.calcWorldPos()), pb.getScale());

			for (auto& e : enemy)
			{
				if (e->getAlive()
					&& Collision::CheckHit(pBulCol,
										   Sphere(XMLoadFloat3(&e->calcWorldPos()),
												  e->getScale())))
				{
					// パーティクルを生成
					XMFLOAT3 pos = e->calcWorldPos();
					createParticle(pos, 98U, 32.f, 16.f);
					// 敵も自機弾もさよなら
					pb.kill();
					e->damage(1u, true);

					Sound::SoundPlayWave(killSe.get(), 0, 0.2f);
				}
			}
		}

		// --------------------
		// 自機と敵弾の当たり判定
		// --------------------
		if (player->getAlive())
		{
			const Sphere playerCol(XMLoadFloat3(&player->calcWorldPos()), player->getScale());

			for (auto& e : enemy)
			{
				for (auto& eb : e->getBulList())
				{
					//　存在しない敵弾の処理はしない
					if (!eb->getAlive()) { continue; }

					// 自機と敵の弾が当たっていたら
					if (Collision::CheckHit(playerCol,
											Sphere(XMLoadFloat3(&eb->calcWorldPos()),
												   eb->getScaleF3().z)))
					{
						// 当たった敵弾は消す
						eb->kill();

						// HPが無くなったら次のシーンへ進む
						if (player->damage(1u, true))
						{
							changeNextScene<GameOverScene>();
							player->kill();
						} else
						{
							// 演出開始
							startRgbShift();
						}
					}
				}
			}
		}

		// ------------------------------
		// 弾がなく、かつ死んだ敵の判定
		// ------------------------------

		// まだ出ていない敵がいなければ
		if (enemyPopData.empty())
		{
			bool enemyEmpty = false;

			// 敵とその弾がすべて消えたかを調べる
			for (const auto& i : enemy)
			{
				// 生きていない && 弾がない
				enemyEmpty = !i->getAlive() && i->bulEmpty();

				// 生きている敵(及びその弾)がいるなら走査終了
				if (!enemyEmpty) { break; }
			}

			// 敵がすべて消えたら次のシーンへ
			if (enemyEmpty)
			{
				startExitPlayer();
			}
		}
	}

	// 自機の体力バーの大きさを変更
	hpBar->setSize(XMFLOAT2((float)player->getHp() / (float)playerHpMax * hpBarWidMax,
							hpBar->getSize().y));

	// ライトはカメラの位置にする
	light->setLightPos(camera->getEye());

	// 今のフレームを進める
	++nowFrame;

	updateRgbShift();
}

void RailShoot::update_exitPlayer()
{
	const auto nowTime = exitPlayer->timer->getNowTime();

	if (nowTime >= exitPlayer->exitTime)
	{
		endExitPlayer();
		return;
	}

	const float raito = (float)nowTime / (float)exitPlayer->exitTime;

	player->setPos(lerp(exitPlayer->playerPos.start, exitPlayer->playerPos.end, raito));

	player->setScaleF3(lerp(exitPlayer->playerScale.start, exitPlayer->playerScale.end, raito));

	camera->setTarget(player->calcWorldPos());
}

template<class NextScene>
void RailShoot::update_end()
{
	const Timer::timeType nowTime = timer->getNowTime() - startSceneChangeTime;

	// 時間が来たら次のシーンへ進む
	if (nowTime >= sceneChangeTime)
	{
		// bgm止める
		Sound::SoundStopWave(bgm.get());

		// 次のシーンへ進む
		SceneManager::getInstange()->changeScene<NextScene>();
	}

	// --------------------
	// 進行度に合わせてポストエフェクトをかける
	// --------------------

	const float timeRaito = (float)nowTime / sceneChangeTime;
	PostEffect::getInstance()->setAlpha(1.f - timeRaito);

	const float mosCoe = std::pow(1.f - timeRaito, 5.f);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::window_width * mosCoe,
													 WinAPI::window_height * mosCoe));

	PostEffect::getInstance()->setNoiseIntensity(1.f - timeRaito);
}

void RailShoot::initFixedCam(const XMFLOAT3& startPos,
							 const XMFLOAT3& endPos)
{
	XMFLOAT3 eye = lerp(startPos, endPos, 0.5f);
	eye.x += player->getScaleF3().x * 1.5f;
	camera->setEye(eye);
	camera->setParentObj(nullptr);
	camera->setTarget(startPos);
}

void RailShoot::startAppearPlayer()
{
	// 登場演出の情報
	appearPlayer = std::make_unique<AppearPlayer>(
		AppearPlayer{
		.playerPos =
			{
				.start = appearPPosStart,
				.end = appearPPosEnd
			},
		.appearTime = Timer::oneSec * 3,
		.timer = std::make_unique<Timer>(),
		.playerScale =
			{
				.start = XMFLOAT3(0,0,0),
				.end = player->getScaleF3()
			},
		.camFogRad =
		{
			.start = camFogStart,
			.end = camFogEnd
}
		}
	);

	// カメラ設定
	initFixedCam(appearPPosStart, appearPPosEnd);

	// 自機の大きさ
	player->setScale(0.f);

	// 自機を演出開始位置に置く
	player->setPos(appearPPosStart);

	update_proc = std::bind(&RailShoot::update_appearPlayer, this);
	appearPlayer->timer->reset();
}

void RailShoot::endAppearPlayer()
{
	// 自機を演出終了位置に置く
	player->setPos(appearPlayer->playerPos.end);

	// 自機の大きさを戻す
	player->setScaleF3(appearPlayer->playerScale.end);

	// レールに追従する
	camera->setParentObj(railObj.get());

	// フォグの設定
	camera->setFogAngleYRad(appearPlayer->camFogRad.end);

	// 操作説明を表示
	for (auto& i : operInst)
	{
		i.second->isInvisible = false;
	}

	// 照準を表示
	aim2D->isInvisible = false;

	update_proc = std::bind(&RailShoot::update_play, this);
	appearPlayer.reset(nullptr);
}

void RailShoot::startExitPlayer()
{
	aim2D->isInvisible = true;
	for (auto& i : operInst)
	{
		if (!i.second->isInvisible) { i.second->isInvisible = true; }
	}

	exitPlayer = std::make_unique<ExitPlayer>(ExitPlayer
											  {
												  .playerPos = {
													  .start = player->getPos(),
													  .end = XMFLOAT3(player->getPos().x, player->getPos().y + 1000.f, player->getPos().z + 1000.f)
												  },
												  .exitTime = Timer::oneSec * 3,
												  .timer = std::make_unique<Timer>(),
												  .playerScale = {
													  .start = player->getScaleF3(),
													  .end = XMFLOAT3(0.f, 0.f, 0.f)
												  }
											  });
	camera->setParentObj(nullptr);

	update_proc = std::bind(&RailShoot::update_exitPlayer, this);
	exitPlayer->timer->reset();
}

void RailShoot::endExitPlayer()
{
	exitPlayer.reset(nullptr);
	changeNextScene<BossScene>();
}

void RailShoot::updateRailPos()
{
	float raito = float(splineNowFrame++) / float(splineFrameMax);
	if (raito >= 1.f)
	{
		if (splineIndex >= splinePoint.size() - 3u)
		{
			raito = 1.f;
		} else
		{
			++splineIndex;
			raito -= 1.f;
			splineNowFrame = 0u;
		}
	}
	// 更新前の位置を記憶
	const XMFLOAT3 prePos = railObj->getPos();

	// 更新後の位置を算出
	XMFLOAT3 pos{};
	XMStoreFloat3(&pos,
				  splinePosition(splinePoint,
								 splineIndex,
								 raito));
	// 位置を反映
	railObj->setPos(pos);

	if (pos.x != prePos.x ||
		pos.y != prePos.y ||
		pos.z != prePos.z)
	{
		// 移動速度を算出
		const XMFLOAT3 vel = XMFLOAT3(pos.x - prePos.x,
									  pos.y - prePos.y,
									  pos.z - prePos.z);

		// 移動方向への回転角
		XMFLOAT2 rota = GameObj::calcRotationSyncVelDeg(vel);

		// 異常な値は0にする
		if (!isfinite(rota.x)) { rota.x = 0.f; }
		if (!isfinite(rota.y)) { rota.y = 0.f; }

		// 回転を反映
		railObj->setRotation(XMFLOAT3(rota.x, rota.y, railObj->getRotation().z));
	}
}

// --------------------
// 自機移動回転
// --------------------
void RailShoot::movePlayer()
{
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

	auto playerRot = XMFLOAT3(0.f, player->getRotation().y, 0.f);

	if (moveXFlag || moveYFlag)
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

		XMFLOAT2 moveVal = XMFLOAT2(0.f, 0.f);

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
			bool moveFlag = false;
			if (inputVal.y > 0.f)
			{
				if (player->getPos().y <= 140.f)
				{
					moveFlag = true;
					operInst.at("W")->isInvisible = true;
				}
			} else if (inputVal.y < 0.f)
			{
				if (player->getPos().y >= -55.f)
				{
					moveFlag = true;
					operInst.at("S")->isInvisible = true;
				}
			}
			if (moveFlag)
			{
				playerRot.x = 45.f * -inputVal.y;
				moveVal.y += inputVal.y * speed;
			}
		}
		if (moveXFlag)
		{
			bool moveFlag = false;
			if (inputVal.x > 0.f)
			{
				if (player->getPos().x <= 140.f)
				{
					moveFlag = true;
					operInst.at("D")->isInvisible = true;
				}
			} else if (inputVal.x < 0.f)
			{
				if (player->getPos().x >= -140.f)
				{
					moveFlag = true;
					operInst.at("A")->isInvisible = true;
				}
			}
			if (moveFlag)
			{
				playerRot.z = 45.f * -inputVal.x;
				moveVal.x += inputVal.x * speed;
			}
		}

		XMFLOAT3 pos = player->getPos();
		pos.x += moveVal.x;
		pos.y += moveVal.y;
		player->setPos(pos);
	}
	player->setRotation(playerRot);
}

void RailShoot::updatePlayerShotTarget()
{
	// 照準の範囲
	const XMFLOAT2 aim2DMin = XMFLOAT2(input->getMousePos().x - aim2D->getSize().x / 2.f,
									   input->getMousePos().y - aim2D->getSize().y / 2.f);
	const XMFLOAT2 aim2DMax = XMFLOAT2(input->getMousePos().x + aim2D->getSize().x / 2.f,
									   input->getMousePos().y + aim2D->getSize().y / 2.f);

	// スクリーン上の敵の位置格納変数
	XMFLOAT2 screenEnemyPos{};

	// 遠い敵を調べるためのもの
	float nowEnemyDistance{};
	std::weak_ptr<NormalEnemy> farthestEnemyPt;
	float farthestEnemyLen = 1.f;

	// 照準の中の敵の方へ弾を飛ばす
	for (auto& i : enemy)
	{
		// いない敵は無視
		if (!i->getAlive()) { continue; }

		// 敵のスクリーン座標を取得
		screenEnemyPos = i->getObj()->calcScreenPos();

		// 敵が2D照準の中にいるかどうか
		if (aim2DMin.x <= screenEnemyPos.x &&
			aim2DMin.y <= screenEnemyPos.y &&
			aim2DMax.x >= screenEnemyPos.x &&
			aim2DMax.y >= screenEnemyPos.y)
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
	} else
	{
		player->setShotTarget(farthestEnemyPt);
	}
}

void RailShoot::updateAimCol()
{
	// 照準の範囲
	const XMFLOAT2 aim2DMin = XMFLOAT2(input->getMousePos().x - aim2D->getSize().x / 2.f,
									   input->getMousePos().y - aim2D->getSize().y / 2.f);
	const XMFLOAT2 aim2DMax = XMFLOAT2(input->getMousePos().x + aim2D->getSize().x / 2.f,
									   input->getMousePos().y + aim2D->getSize().y / 2.f);

	// スクリーン上の敵の位置格納変数
	XMFLOAT2 screenEnemyPos{};

	// 照準内に敵がいない時の色
	aim2D->color = XMFLOAT4(1, 1, 1, 1);

	for (auto& i : enemy)
	{
		// いない敵は無視
		if (!i->getAlive()) { continue; }

		// 敵のスクリーン座標を取得
		screenEnemyPos = i->getObj()->calcScreenPos();

		// 敵が2D照準の中にいるかどうか
		if (aim2DMin.x <= screenEnemyPos.x &&
			aim2DMin.y <= screenEnemyPos.y &&
			aim2DMax.x >= screenEnemyPos.x &&
			aim2DMax.y >= screenEnemyPos.y)
		{
			// 敵がいれば色を変える
			aim2D->color = XMFLOAT4(1, 0, 0, 1);
			break;
		}
	}
}

void RailShoot::drawObj3d()
{
	backObj->drawWithUpdate(light.get(), backPipelineSet);

	groundObj->drawWithUpdate(light.get());
	player->drawWithUpdate(light.get());
	for (auto& i : enemy)
	{
		i->drawWithUpdate(light.get());
	}

	for (auto& y : laneWall)
	{
		for (auto& x : y)
		{
			x->drawWithUpdate(light.get());
		}
	}

	particleMgr->drawWithUpdate();
}

void RailShoot::drawFrontSprite()
{
	// 最初のウインドウの位置を指定
	constexpr XMFLOAT2 fstWinPos = XMFLOAT2((float)WinAPI::window_width * 0.02f,
											(float)WinAPI::window_height * 0.02f);
	ImGui::SetNextWindowPos(ImVec2(fstWinPos.x,
								   fstWinPos.y));
	ImGui::SetNextWindowSize(ImVec2(256.f, 128.f));

	ImGui::Begin("レールシューティング", nullptr, DX12Base::imGuiWinFlagsDef);
	ImGui::Text("");
	ImGui::Text("自機体力 : %.2f%%(%u / %u)",
				(float)player->getHp() / (float)playerHpMax * 100.f,
				player->getHp(),
				playerHpMax);
	ImGui::End();

	spriteBase->drawStart(dxBase->getCmdList());

	hpBarEdge->drawWithUpdate(dxBase, spriteBase.get());
	hpBar->drawWithUpdate(dxBase, spriteBase.get());

	aim2D->drawWithUpdate(dxBase, spriteBase.get());

	for (auto& i : operInst)
	{
		i.second->drawWithUpdate(DX12Base::ins(), spriteBase.get());
	}

	debugText->DrawAll(dxBase, spriteBase.get());
}

RailShoot::~RailShoot()
{
	PostEffect::getInstance()->setAlpha(1.f);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::window_width,
													 WinAPI::window_height));
	PostEffect::getInstance()->setRgbShiftNum({ 0.f, 0.f });
	PostEffect::getInstance()->setSpeedLineIntensity(0.f);

	PostEffect::getInstance()->setVignIntensity(0.25f);
	PostEffect::getInstance()->changePipeLine(0U);
}