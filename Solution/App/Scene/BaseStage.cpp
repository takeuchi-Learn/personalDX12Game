#include "BaseStage.h"
#include "System/SceneManager.h"
#include "TitleScene.h"
#include <fstream>
#include <DirectXMath.h>

using namespace DirectX;

#pragma region 初期化

BaseStage::BaseStage() :
	dxBase(DX12Base::ins()),
	input(Input::getInstance()),
	timer(std::make_unique<Timer>()),
	camera(std::make_unique<CameraObj>(nullptr)),
	light(std::make_unique<Light>()),

	update_proc(std::bind(&BaseStage::update_start, this)),
	particleMgr(std::make_unique<ParticleMgr>(L"Resources/effect1.png", camera.get()))
{
	// カメラ
	camera->setFarZ(10000.f);

	// 自機
	initPlayer();

	// 天球と地面
	initBackObj();

	// スプライト
	initSprite();
}

void BaseStage::initPlayer()
{
	playerModel = std::make_unique<ObjModel>("Resources/player", "player");
	playerBulModel = std::make_unique<ObjModel>("Resources/bullet", "bullet", 0U, true);
	playerHpMax = 20U;

	player = std::make_unique<Player>(camera.get(), playerModel.get(), XMFLOAT3(0.f, 0.f, 0.f));
	// 大きさを設定
	player->setScale(10.f);
}

void BaseStage::initBackObj()
{
	backPipelineSet = Object3d::createGraphicsPipeline(Object3d::BLEND_MODE::ALPHA,
													   L"Resources/Shaders/BackVS.hlsl",
													   L"Resources/Shaders/BackPS.hlsl");

	// 背景の天球
	back = std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true);
	const float backScale = camera->getFarZ() * 0.9f;
	back->setScale({ backScale, backScale, backScale });

	// 地面
	ground = std::make_unique<ObjSet>(camera.get(), "Resources/ground", "ground");
	constexpr UINT groundSize = 5000u;
	ground->setPos(XMFLOAT3(0, -player->getScale() * 5.f, 0));
	ground->setScale(XMFLOAT3(groundSize, groundSize, groundSize));

	constexpr float tillingNum = (float)groundSize / 32.f;
	ground->getModelPt()->setTexTilling(XMFLOAT2(tillingNum, tillingNum));
}

void BaseStage::initSprite()
{
	spBase = std::make_unique<SpriteBase>();

	aim2D = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/aimPos.png"),
									 spBase.get());
}

void BaseStage::start()
{
	// マウスカーソルは表示しない
	input->changeDispMouseCursorFlag(false);
	timer->reset();
}

#pragma endregion 初期化

void BaseStage::update_start()
{
	update_proc = std::bind(&BaseStage::update_play, this);

	// 自機に追従する
	camera->setParentObj(player.get());
}

void BaseStage::update_play()
{
	movePlayer();
}

void BaseStage::update_end()
{
	SceneManager::getInstange()->changeScene<TitleScene>();
}

void BaseStage::movePlayer()
{
	// 移動
	{
		const bool hitW = input->hitKey(DIK_W);
		const bool hitS = input->hitKey(DIK_S);

		if (hitW || hitS)
		{
			float moveSpeed = 90.f / DX12Base::ins()->getFPS();
			if (input->hitKey(DIK_LCONTROL))
			{
				moveSpeed /= 2.f;
			} else if (input->hitKey(DIK_LSHIFT))
			{
				moveSpeed *= 2.f;
			}

			if (hitW)
			{
				player->moveForward(moveSpeed);
			} else if (hitS)
			{
				player->moveForward(-moveSpeed);
			}
		}
	}

	// 回転
	{
		const bool hitA = input->hitKey(DIK_A);
		const bool hitD = input->hitKey(DIK_D);

		if (hitA || hitD)

			// 左右の回転
			if (hitA || hitD)
			{
				float speed = 45.f / DX12Base::ins()->getFPS();

				// 左シフトと左コントロールで速度変更
				if (input->hitKey(DIK_LCONTROL))
				{
					speed /= 2.f;
				} else if (input->hitKey(DIK_LSHIFT))
				{
					speed *= 2.f;
				}

				XMFLOAT3 rota = player->getRotation();

				// 回転させる
				if (hitA)
				{
					rota.y -= speed;
				} else if (hitD)
				{
					rota.y += speed;
				}

				player->setRotation(rota);
			}
	}
}

BaseStage::CSVType BaseStage::loadCsv(const std::string& csvFilePath,
									  bool commentFlag,
									  char divChar,
									  const std::string& commentStartStr)
{
	CSVType csvData{};	// csvの中身を格納

	std::ifstream ifs(csvFilePath);
	if (!ifs) { return csvData; }

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

void BaseStage::update()
{
	{
		// シーン遷移中も背景は回す
		XMFLOAT2 shiftUv = back->getModelPt()->getShiftUv();
		constexpr float shiftSpeed = 0.01f;

		shiftUv.x += shiftSpeed / DX12Base::getInstance()->getFPS();

		back->getModelPt()->setShivtUv(shiftUv);
	}

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
	aim2D->position.x = player->getAim2DPos().x;
	aim2D->position.y = player->getAim2DPos().y;

	// 更新処理本体
	update_proc();

	// 背景オブジェクトの中心をカメラにする
	back->setPos(camera->getEye());
	// ライトはカメラの位置にする
	light->setLightPos(camera->getEye());

	// ライトとカメラの更新
	light->update();
	camera->update();
}

void BaseStage::drawObj3d()
{
	Object3d::startDraw(backPipelineSet);
	back->drawWithUpdate(light.get());

	Object3d::startDraw();
	ground->drawWithUpdate(light.get());

	player->drawWithUpdate(light.get());

	for (auto& i : attackableEnemy)
	{
		i->drawWithUpdate(light.get());
	}

	additionalDrawObj3d();

	particleMgr->drawWithUpdate();
}

void BaseStage::drawFrontSprite()
{
	ImGui::SetNextWindowPos(ImVec2(fstWinPos.x,
								   fstWinPos.y));
	ImGui::SetNextWindowSize(ImVec2(200.f, 100.f));

	ImGui::Begin("テンプレ", nullptr, winFlags);
	ImGui::Text("テンプレ");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x,
								   ImGui::GetWindowPos().y + ImGui::GetWindowSize().y));
	ImGui::End();
}

BaseStage::~BaseStage()
{
}