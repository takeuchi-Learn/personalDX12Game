#include "RailShoot.h"
#include <DirectXMath.h>

#include "SceneManager.h"
#include "BossScene.h"

#include "PostEffect.h"

#include "Collision.h"

#include "RandomNum.h"

#include <fstream>

using namespace DirectX;

// std::stringの2次元配列(vector)
using CSVType = std::vector<std::vector<std::string>>;
// @brief loadCsvの入力をstd::stringにしたもの
// @return 読み込んだcsvの中身。失敗したらデフォルトコンストラクタで初期化された空のvector2次元配列が返る
// @param commentFlag //で始まる行を無視するかどうか(trueで無視)
// @param divChar フィールドの区切り文字
// @param commentStartStr コメント開始文字
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

	timer(std::make_unique<Time>()),

	spriteBase(std::make_unique<SpriteBase>(SpriteBase::BLEND_MODE::ALPHA)),

	// --------------------
	// 背景と地面
	// --------------------
	back(std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true)),
	ground(std::make_unique<ObjSet>(camera.get(), "Resources/ground", "ground", false)),

	// --------------------
	// 敵モデル
	// --------------------
	enemyModel(std::make_unique<ObjModel>("Resources/enemy", "enemy", 0U, true)),
	enemyBulModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),

	// --------------------
	// 自機関連
	// --------------------
	playerModel(std::make_unique<ObjModel>("Resources/player", "player")),
	playerBulModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),
	playerHpMax(20u),
	playerHp(playerHpMax),

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
					 XMFLOAT2(0.5f, 1.f))),
	hpBarWidMax(WinAPI::window_width * 0.75f)
{
	hpBar->color = XMFLOAT4(1, 1, 1, 0.75f);
	hpBar->position = XMFLOAT3(WinAPI::window_width / 2.f, WinAPI::window_height, 0.f);
	{
		XMFLOAT2 size = hpBar->getSize();
		size.x = hpBarWidMax;
		size.y = (float)WinAPI::window_height / 32.f;
		hpBar->setSize(size);

		hpBar->position.y -= size.y;
	}

	// --------------------
	// カメラ初期化
	// --------------------
	camera->setFarZ(5000.f);
	camera->setEye(XMFLOAT3(0, WinAPI::getInstance()->getWindowSize().y * 0.06f, -180.f));	// 視点座標
	camera->setTarget(XMFLOAT3(0, 0, 0));	// 注視点座標
	camera->setUp(XMFLOAT3(0, 1, 0));		// 上方向
	//camera->update();

	// --------------------
	// ライト初期化
	// --------------------
	light->setLightPos(camera->getEye());

	// --------------------
	// レール現在位置のオブジェクト
	// --------------------
	/*railObj->setPos(XMFLOAT3(0, 0, 0));
	railObj->setScale(1.f);
	railObj->setRotation(XMFLOAT3(0, 0, 0));*/

	// --------------------
	// 自機初期化
	// --------------------
	player = std::make_unique<Player>(camera.get(), playerModel.get());
	player->setScale(10.f);
	player->setParent(railObj.get());
	player->setPos(XMFLOAT3(0, 12.f, 0));

	// カメラをレールに追従させる
	camera->setParentObj(railObj.get());
	camera->update();

	// --------------------
	// スプライン
	// --------------------

	{
		// 制御点の情報はCSVから読み込む
		csvData = loadCsv("Resources/splinePos.csv", true, ',', "//");

		// startは2つ必要
		splinePoint.emplace_back(XMVectorSet(std::stof(csvData[0][0]),
											 std::stof(csvData[0][1]),
											 std::stof(csvData[0][2]),
											 1));

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


	// --------------------
	// 敵初期化
	// --------------------

	// 敵は最初居ない
	enemy.resize(0U);

	// 敵発生スクリプト
	csvData = loadCsv("Resources/enemyScript.csv", true, ',', "//");
	{
		UINT waitFrame = 0u;
		for (auto& y : csvData)
		{
			if (y[0] == "WAIT")
			{
				waitFrame += (UINT)std::stoi(y[1]);
			} else if (y[0] == "PUSH")
			{
				enemyPopData.emplace_front(std::make_unique<PopEnemyData>(waitFrame,
																		  XMFLOAT3(std::stof(y[1]),
																				   std::stof(y[2]),
																				   std::stof(y[3])),
																		  XMFLOAT3(0, 0, -1)));
			}
		}
	}

	// --------------------
	// 背景と地面
	// --------------------

	// 背景の天球
	const float backScale = camera->getFarZ() * 0.9f;
	back->setScale({ backScale, backScale, backScale });

	// 地面
	constexpr UINT groundSize = 5000u;
	ground->setPos(XMFLOAT3(0, -player->getScale(), (float)groundSize));
	ground->setScale(XMFLOAT3(groundSize, groundSize, groundSize));
	ground->getModelPt()->setTexTilling(XMFLOAT2(groundSize / 32.f, groundSize / 32.f));

	// --------------------
	// マウスカーソルは表示しない
	// --------------------
	input->changeDispMouseCursorFlag(false);
}

void RailShoot::start()
{
	// タイマー開始
	timer->reset();
	startSceneChangeTime = timer->getNowTime();
}

void RailShoot::update()
{
	{
		// シーン遷移中も背景は回す
		XMFLOAT2 shiftUv = back->getModelPt()->getShiftUv();
		constexpr float shiftSpeed = 0.01f;

		shiftUv.x += shiftSpeed / DX12Base::getInstance()->getFPS();

		back->getModelPt()->setShivtUv(shiftUv);
	}

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
							   const UINT particleNum,
							   const float startScale,
							   const float vel)
{
	for (UINT i = 0U; i < particleNum; ++i)
	{
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

		const XMFLOAT3 particlePos{
			pos.x + railObj->getPos().x,
			pos.y + railObj->getPos().y,
			pos.z + railObj->getPos().z
		};

		// 追加
		particleMgr->add(std::make_unique<Time>(),
						 life, particlePos, vel, acc,
						 startScale, endScale,
						 startRota, endRota,
						 startCol, endCol);
	}
}

void RailShoot::startRgbShift()
{
	rgbShiftFlag = true;
	nowRgbShiftTime = 0;
	startRgbShiftTime = timer->getNowTime();
}

void RailShoot::updateRgbShift()
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

		constexpr float rgbShiftMumMax = 1.f / 16.f;

		constexpr float  c4 = 2.f * XM_PI / 3.f;
		const float easeRate = -powf(2.f, 10.f * (1.f - raito) - 10.f) *
			dxBase->nearSin((raito * 10.f - 10.75f) * c4);

		PostEffect::getInstance()->setRgbShiftNum({ easeRate * rgbShiftMumMax, 0.f });
	}
}

void RailShoot::addEnemy(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& vel, float scale)
{
	auto& i = enemy.emplace_front(new Enemy(camera.get(), enemyModel.get(), enemyBulModel.get(), pos));
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
	const Time::timeType nowTime = timer->getNowTime() - startSceneChangeTime;
	if (nowTime >= sceneChangeTime)
	{
		update_proc = std::bind(&RailShoot::update_play, this);
	}

	const float timeRaito = (float)nowTime / sceneChangeTime;
	PostEffect::getInstance()->setAlpha(timeRaito);

	const float mosCoe = powf(timeRaito, 5);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::getInstance()->getWindowSize().x * mosCoe,
													 WinAPI::getInstance()->getWindowSize().y * mosCoe));

	PostEffect::getInstance()->setNoiseIntensity(1.f - timeRaito);

	const float vignNum = 0.5f * timeRaito;
	PostEffect::getInstance()->setVignIntensity(vignNum);

	const float speedLineIntensity = 0.125f * timeRaito;
	PostEffect::getInstance()->setSpeedLineIntensity(speedLineIntensity);
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

	// --------------------
	// 敵を増やす
	// --------------------

	// 終わった発生情報は削除
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

	// Z座標が0を超えた敵は退場
	for (auto& i : enemy)
	{
		if (i->getPos().z < 0.f)
		{
			i->chansePhase_Leave(DirectX::XMFLOAT3(-1, 1, 0));
		}
	}

	// --------------------
	// 自機の弾発射
	// --------------------
	playerShot();

	// --------------------
	// 弾発射
	// --------------------
	if (player->shotTargetIsEmpty())
	{
		if (input->triggerMouseButton(Input::MOUSE::LEFT))
		{
			constexpr float bulSpeed = 8.f;
			player->shot(camera.get(), playerBulModel.get(), bulSpeed);
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
			if (!pb.getAlive()) continue;

			pBulCol = Sphere(XMLoadFloat3(&pb.getPos()), pb.getScale());

			for (auto& e : enemy)
			{
				if (e->getAlive()
					&& Collision::CheckHit(pBulCol,
										   Sphere(XMLoadFloat3(&e->getPos()),
												  e->getScale())))
				{
					// パーティクルを生成
					createParticle(e->getPos(), 98U, 32.f, 16.f);
					// 敵も自機弾もさよなら
					e->kill();
					pb.kill();
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
					if (!player->getAlive()) break;

					//　存在しない敵弾の処理はしない
					if (!eb->getAlive()) continue;

					// 自機と敵の弾が当たっていたら
					if (Collision::CheckHit(playerCol,
											Sphere(XMLoadFloat3(&eb->getPos()),
												   eb->getScaleF3().z)))
					{
						// 当たった敵弾は消す
						eb->kill();
						// HPが無くなったら次のシーンへ進む
						if (--playerHp <= 0u)
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

		// 弾がなく、かつ死んだ敵は消す
		enemy.remove_if([](const std::unique_ptr<Enemy>& i) { return !i->getAlive() && i->bulEmpty(); });

		// 敵がすべて消えたら次のシーンへ
		if (enemy.empty() && enemyPopData.empty())
		{
			changeNextScene();
		}
	}

	// ライトはカメラの位置にする
	light->setLightPos(camera->getEye());

	// 今のフレームを進める
	++nowFrame;

	updateRgbShift();
}

void RailShoot::update_end()
{
	const Time::timeType nowTime = timer->getNowTime() - startSceneChangeTime;

	// 時間が来たら次のシーンへ進む
	if (nowTime >= sceneChangeTime)
	{
		SceneManager::getInstange()->changeScene(new BossScene());
	}

	// --------------------
	// 進行度に合わせてポストエフェクトをかける
	// --------------------

	const float timeRaito = (float)nowTime / sceneChangeTime;
	PostEffect::getInstance()->setAlpha(1.f - timeRaito);

	const float mosCoe = powf(1.f - timeRaito, 5);
	PostEffect::getInstance()->setMosaicNum(XMFLOAT2(WinAPI::window_width * mosCoe,
													 WinAPI::window_height * mosCoe));

	PostEffect::getInstance()->setNoiseIntensity(1.f - timeRaito);
}

void RailShoot::updateRailPos()
{
	float raito = float(splineNowFrame++) / float(splineFrameMax);
	if (raito >= 1.f)
	{
		if (splineIndex < splinePoint.size() - 3)
		{
			++splineIndex;
			raito -= 1.f;
			splineNowFrame = 0u;
		} else
		{
			raito = 1.f;
		}
	}
	XMFLOAT3 pos{};
	XMStoreFloat3(&pos,
				  splinePosition(splinePoint,
								 splineIndex,
								 raito));
	railObj->setPos(pos);
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
		} else if (hitA && player->getPos().x > -110.f)
		{
			player->moveRight(-moveSpeed);
		}

		// 高さ方向に移動
		if (hitW && player->getPos().y < 110.f)
		{
			player->moveUp(moveSpeed);
		} else if (hitS && player->getPos().y > 5.f)
		{
			player->moveUp(-moveSpeed);
		}
	}
}

void RailShoot::playerShot()
{
	// 照準の範囲
	const XMFLOAT2 aim2DMin = XMFLOAT2(input->getMousePos().x - aim2D->getSize().x / 2.f,
									   input->getMousePos().y - aim2D->getSize().y / 2.f);
	const XMFLOAT2 aim2DMax = XMFLOAT2(input->getMousePos().x + aim2D->getSize().x / 2.f,
									   input->getMousePos().y + aim2D->getSize().y / 2.f);

	// スクリーン上の敵の位置格納変数
	XMFLOAT2 screenEnemyPos{};

	// 遠い敵を調べるためのもの
	float oldEnemyDistance{}, nowEnemyDistance{};
	Enemy* farthestEnemyPt = nullptr;
	float farthestEnemyLen = 1.f;

	// 最も近い敵の方へ弾を飛ばす
	for (auto& i : enemy)
	{
		// いない敵は無視
		if (!i->getAlive()) continue;

		// 敵のスクリーン座標を取得
		screenEnemyPos = i->getObj()->calcScreenPos();

		// 敵が2D照準の中にいるかどうか
		if (aim2DMin.x <= screenEnemyPos.x &&
			aim2DMin.y <= screenEnemyPos.y &&
			aim2DMax.x >= screenEnemyPos.x &&
			aim2DMax.y >= screenEnemyPos.y)
		{
			// 敵との距離を更新
			oldEnemyDistance = nowEnemyDistance;
			nowEnemyDistance = sqrtf(
				powf(i->getPos().x - camera->getEye().x, 2.f) +
				powf(i->getPos().y - camera->getEye().y, 2.f) +
				powf(i->getPos().z - camera->getEye().z, 2.f)
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
		player->setShotTarget(farthestEnemyPt->getObj());
		aim2D->color = XMFLOAT4(1, 0, 0, 1);
	} else
	{
		player->setShotTarget(nullptr);
		aim2D->color = XMFLOAT4(0, 0, 0, 1);
	}
#ifdef _DEBUG
	// 照準の中に敵がいるかどうかを表示
	debugText->formatPrint(spriteBase.get(),
						   300.f, 0.f,
						   1.f,
						   { 1,1,1,1 },
						   "%s", farthestEnemyPt != nullptr ? "IN" : "NO_ENEMY");
#endif // _DEBUG
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

	particleMgr->drawWithUpdate();
}

void RailShoot::drawFrontSprite()
{
	constexpr ImGuiWindowFlags winFlags =
		// リサイズ不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		// タイトルバー無し
		//ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
		// 設定を.iniに出力しない
		ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings |
		// 移動不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove;
	//// スクロールバーを常に表示
	//ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysHorizontalScrollbar |
	//ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar;

// 最初のウインドウの位置を指定
	constexpr XMFLOAT2 fstWinPos = XMFLOAT2((float)WinAPI::window_width * 0.02f,
											(float)WinAPI::window_height * 0.02f);
	ImGui::SetNextWindowPos(ImVec2(fstWinPos.x,
								   fstWinPos.y));

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
	ImGui::Text("自機体力 : %u", playerHp);
	ImGui::End();

	spriteBase->drawStart(dxBase->getCmdList());

	hpBar->setSize(XMFLOAT2((float)playerHp / (float)playerHpMax * hpBarWidMax,
							hpBar->getSize().y));
	hpBar->drawWithUpdate(dxBase, spriteBase.get());

	aim2D->position = XMFLOAT3(player->getAim2DPos().x, player->getAim2DPos().y, 0.f);
	aim2D->drawWithUpdate(dxBase, spriteBase.get());

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