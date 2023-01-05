#include "RailShoot.h"
#include <DirectXMath.h>

#include "BossScene.h"

#include <fstream>
#include "Util/RandomNum.h"
#include "System/PostEffect.h"
#include "Collision/Collision.h"
#include "System/SceneManager.h"

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
	// 背景と地面
	// --------------------
	back(std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true)),
	ground(std::make_unique<ObjSet>(camera.get(), "Resources/ground", "ground", false)),

	// --------------------
	// 敵モデル
	// --------------------
	enemyModel(std::make_unique<ObjModel>("Resources/tori", "tori", 0U, true)),
	enemyBulModel(std::make_unique<ObjModel>("Resources/tree", "tree", 0U, true)),

	// --------------------
	// 自機関連
	// --------------------
	playerModel(std::make_unique<ObjModel>("Resources/player", "player")),
	playerBulModel(std::make_unique<ObjModel>("Resources/bullet", "bullet", 0U, true)),
	playerHpMax(20u),

	// --------------------
	// レール現在位置のオブジェクト
	// --------------------
	railObj(std::make_unique<GameObj>(camera.get(), nullptr)),

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

	// --------------------
	// カメラ初期化
	// --------------------
	camera->setFarZ(5000.f);
	camera->setEye(XMFLOAT3(0, WinAPI::getInstance()->getWindowSize().y * 0.06f, -180.f));	// 視点座標
	camera->setTarget(XMFLOAT3(0, 0, 0));	// 注視点座標
	camera->setUp(XMFLOAT3(0, 1, 0));		// 上方向
	//camera->setEye2TargetLen(50.f);
	{
		/*XMFLOAT3 rota = camera->getRelativeRotaDeg();
		camera->setRelativeRotaDeg(rota);*/
		camera->setFogAngleYRad(XM_PI / 6.f);
	}

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

		// 制御点の数だけオブジェクトを置く
		const size_t splinePointNum = splinePoint.size() - 2u;
		const size_t wallNum = splinePointNum * 2u;
		laneWall.resize(wallNum);

		XMFLOAT3 dest{};
		for (UINT y = 0u; y < wallNum; ++y)
		{
			laneWall[y].first.reset(new Object3d(camera.get(), wallModel.get(), wallModelTexNum));
			laneWall[y].second.reset(new Object3d(camera.get(), wallModel.get(), wallModelTexNum));

			// --------------------
			// オブジェクトの大きさを変更
			// --------------------
			constexpr float scale = 16.f;
			laneWall[y].first->scale = XMFLOAT3(scale, scale * 32.f, scale);
			laneWall[y].second->scale = laneWall[y].first->scale;

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

			// スプライン補間でレーンの位置を求める
			XMStoreFloat3(&dest, splinePosition(splinePoint, (size_t)startIndex, startIndexRaito));
			laneWall[y].first->position = dest;
			laneWall[y].second->position = laneWall[y].first->position;

			// --------------------
			// レーンの左右に配置する
			// --------------------
			constexpr float laneR = 128.f;
			laneWall[y].first->position.x += laneR;
			laneWall[y].second->position.x -= laneWall[y].first->position.x;

			laneWall[y].first->color = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f);
			laneWall[y].second->color = laneWall[y].first->color;
		}
	}

	// --------------------
	// 敵初期化
	// --------------------

	// 敵は最初居ない
	enemy.resize(0U);

	// 敵発生スクリプト
	// todo 関数化
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

	// --------------------
	// 背景と地面
	// --------------------

	// 背景の天球
	{
		const float backScale = camera->getFarZ() * 0.9f;
		back->setScale({ backScale, backScale, backScale });
	}

	// 地面
	{
		constexpr UINT groundSize = 5000u;
		ground->setPos(XMFLOAT3(0, -player->getScale(), (float)groundSize));

		ground->setScale(XMFLOAT3(groundSize, groundSize, groundSize));

		constexpr float tillingNum = groundSize / 32.f;
		ground->getModelPt()->setTexTilling(XMFLOAT2(tillingNum, tillingNum));
	}
}

void RailShoot::start()
{
	// カメラは固定カメラ
	initFixedCam(appearPPosStart, appearPPosEnd);

	// マウスカーソルは表示しない
	input->changeDispMouseCursorFlag(false);

	// タイマー開始
	timer->reset();
	startSceneChangeTime = timer->getNowTime();
}

void RailShoot::update()
{
	rotationBack();

	// 背景オブジェクトの中心をカメラにする
	back->setPos(camera->getEye());

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
	XMFLOAT2 shiftUv = back->getModelPt()->getShiftUv();
	constexpr float shiftSpeed = 0.01f;

	shiftUv.x += shiftSpeed / DX12Base::getInstance()->getFPS();

	back->getModelPt()->setShivtUv(shiftUv);
}

void RailShoot::addEnemy(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& vel, float scale)
{
	auto& i = enemy.emplace_front(new NormalEnemy(camera.get(), enemyModel.get(), enemyBulModel.get(), pos));
	i->setScale(scale);
	i->setVel(vel);
	i->setTargetObj(player.get());
	i->setParent(railObj->getObj());
}

void RailShoot::changeNextScene()
{
	PostEffect::getInstance()->changePipeLine(0U);

	update_proc = std::bind(&RailShoot::update_end, this);
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

	camera->setFogAngleYRad(std::lerp(appearPlayer->camFogRad.start,
									  appearPlayer->camFogRad.end,
									  std::pow(fogRaito, 0.5f)));
}

void RailShoot::update_play()
{
#ifdef _DEBUG

	if (input->hitKey(DIK_LSHIFT) && input->hitKey(DIK_SPACE))
	{
		changeNextScene();
	}

#endif // _DEBUG

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
	if (input->triggerMouseButton(Input::MOUSE::LEFT))
	{
		updatePlayerShotTarget();
		if (player->getShotTarget())
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

			pBulCol = Sphere(XMLoadFloat3(&pb.getPos()), pb.getScale());

			for (auto& e : enemy)
			{
				if (e->getAlive()
					&& Collision::CheckHit(pBulCol,
										   Sphere(XMLoadFloat3(&e->getPos()),
												  e->getScale())))
				{
					// パーティクルを生成
					XMFLOAT3 pos = e->calcWorldPos();
					createParticle(pos, 98U, 32.f, 16.f);
					// 敵も自機弾もさよなら
					pb.kill();
					e->damage(1u, true);
				}
			}
		}

		// --------------------
		// 自機と敵弾の当たり判定
		// --------------------
		if (player->getAlive())
		{
			const Sphere playerCol(XMLoadFloat3(&player->getPos()), player->getScale());

			for (auto& e : enemy)
			{
				for (auto& eb : e->getBulList())
				{
					//　存在しない敵弾の処理はしない
					if (!eb->getAlive()) { continue; }

					// 自機と敵の弾が当たっていたら
					if (Collision::CheckHit(playerCol,
											Sphere(XMLoadFloat3(&eb->getPos()),
												   eb->getScaleF3().z)))
					{
						// 当たった敵弾は消す
						eb->kill();

						// HPが無くなったら次のシーンへ進む
						if (player->damage(1u, true))
						{
							changeNextScene();
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
				changeNextScene();
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

void RailShoot::update_end()
{
	const Timer::timeType nowTime = timer->getNowTime() - startSceneChangeTime;

	// 時間が来たら次のシーンへ進む
	if (nowTime >= sceneChangeTime)
	{
		SceneManager::getInstange()->changeScene<BossScene>();
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
			.start = XM_PI / 9.f,
			.end = XM_PI / 3.f
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

	// 自機に追従する
	camera->setParentObj(player.get());


	camera->setFogAngleYRad(appearPlayer->camFogRad.end);

	// 操作説明を表示
	for (auto& i : operInst)
	{
		i.second->isInvisible = false;
	}

	update_proc = std::bind(&RailShoot::update_play, this);
}

void RailShoot::updateRailPos()
{
	float raito = float(splineNowFrame++) / float(splineFrameMax);
	if (raito >= 1.f)
	{
		if (splineIndex < splinePoint.size() - 3u)
		{
			++splineIndex;
			raito -= 1.f;
			splineNowFrame = 0u;
		} else
		{
			raito = 1.f;
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

// --------------------
// 自機移動回転
// --------------------
void RailShoot::movePlayer()
{
	const bool hitW = input->hitKey(DIK_W);
	const bool hitA = input->hitKey(DIK_A);
	const bool hitS = input->hitKey(DIK_S);
	const bool hitD = input->hitKey(DIK_D);

	if (hitW || hitA || hitS || hitD)
	{
		const float moveSpeed = 90.f / dxBase->getFPS();

		// 横移動
		if (hitD && player->getPos().x < 110.f)
		{
			player->moveRight(moveSpeed);
			operInst.at("D")->isInvisible = true;
		} else if (hitA && player->getPos().x > -110.f)
		{
			player->moveRight(-moveSpeed);
			operInst.at("A")->isInvisible = true;
		}

		// 高さ方向に移動
		if (hitW && player->getPos().y < 110.f)
		{
			player->moveUp(moveSpeed);
			operInst.at("W")->isInvisible = true;
		} else if (hitS && player->getPos().y > 5.f)
		{
			player->moveUp(-moveSpeed);
			operInst.at("S")->isInvisible = true;
		}
	}
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
	NormalEnemy* farthestEnemyPt = nullptr;
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
				farthestEnemyPt = i.get();
				farthestEnemyLen = nowEnemyDistance;
			}
		}
	}

	// 照準の中に敵がいればそこへ弾を出す
	// いなければターゲットはいない
	if (farthestEnemyPt != nullptr)
	{
		player->setShotTarget(farthestEnemyPt);
	} else
	{
		player->setShotTarget(nullptr);
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
	Object3d::startDraw(backPipelineSet);
	back->drawWithUpdate(light.get());

	Object3d::startDraw();
	ground->drawWithUpdate(light.get());
	player->drawWithUpdate(light.get());
	for (auto& i : enemy)
	{
		i->drawWithUpdate(light.get());
	}

	for (auto& y : laneWall)
	{
		y.first->drawWithUpdate(DX12Base::ins(), light.get());
		y.second->drawWithUpdate(DX12Base::ins(), light.get());
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
	ImGui::Text("自機体力 : %u / %u", player->getHp(), playerHpMax);
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