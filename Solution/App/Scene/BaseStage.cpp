#include "BaseStage.h"
#include "../Engine/System/SceneManager.h"
#include "TitleScene.h"
#include <fstream>
#include <DirectXMath.h>

using namespace DirectX;

void BaseStage::update_start()
{
	update_proc = std::bind(&BaseStage::update_play, this);
}

void BaseStage::update_play()
{
	movePlayer();

	additionalUpdate_play();
}

void BaseStage::update_end()
{
	SceneManager::getInstange()->changeScene(new TitleScene());
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

bool BaseStage::playerDamage(UINT damageNum)
{
	if (player->getAlive())
	{
		if (damageNum > playerHp)
		{
			playerHp = 0u;
			player->kill();
			return true;
		}
		playerHp -= damageNum;
	}
	return false;
}

BaseStage::CSVType BaseStage::loadCsv(const std::string& csvFilePath,
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

BaseStage::BaseStage() :
#pragma region シーン内共通

	dxBase(DX12Base::ins()),
	input(Input::getInstance()),
	timer(std::make_unique<Timer>()),
	camera(std::make_unique<CameraObj>(nullptr)),
	light(std::make_unique<Light>()),

	update_proc(std::bind(&BaseStage::update_start, this)),
#pragma  endregion シーン内共通

#pragma region 背景と地面
	backPipelineSet(Object3d::createGraphicsPipeline(Object3d::BLEND_MODE::ALPHA,
													 L"Resources/Shaders/BackVS.hlsl",
													 L"Resources/Shaders/BackPS.hlsl")),
	back(std::make_unique<ObjSet>(camera.get(), "Resources/back/", "back", true)),
	ground(std::make_unique<ObjSet>(camera.get(), "Resources/ground", "ground")),

#pragma endregion 背景と地面

#pragma region 自機関係

	playerModel(std::make_unique<ObjModel>("Resources/player", "player")),
	playerBulModel(std::make_unique<ObjModel>("Resources/sphere", "sphere", 0U, true)),
	playerHpMax(20U),
	playerHp(playerHpMax),

#pragma endregion 自機関係

#pragma region パーティクル

	particleMgr(std::make_unique<ParticleMgr>(L"Resources/effect1.png", camera.get()))

#pragma endregion パーティクル

{
	// --------------------
	// 自機
	// --------------------

	// 初期化子でやるとモデルがnullptrになる
	player = std::make_unique<Player>(camera.get(), playerModel.get(), XMFLOAT3(0.f, 0.f, 0.f));
	// 大きさを設定
	player->setScale(10.f);


	// --------------------
	// 背景と地面
	// --------------------

	// 背景の天球
	const float backScale = camera->getFarZ() * 0.9f;
	back->setScale({ backScale, backScale, backScale });

	// 地面
	const UINT groundSize = 5000u;
	ground->setPos(XMFLOAT3(0, -player->getScale(), 0));
	ground->setScale(XMFLOAT3(groundSize, groundSize, groundSize));
	ground->getModelPt()->setTexTilling(XMFLOAT2(groundSize / 32.f, groundSize / 32.f));

	constexpr float tillingNum = (float)groundSize / 32.f;
	ground->getModelPt()->setTexTilling(XMFLOAT2(tillingNum, tillingNum));

	// --------------------
	// マウスカーソルは表示しない
	// --------------------
	input->changeDispMouseCursorFlag(false);
}

void BaseStage::start()
{
	// 自機に追従する
	camera->setParentObj(player.get());
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

	// 背景オブジェクトの中心をカメラにする
	back->setPos(camera->getEye());
	// ライトはカメラの位置にする
	light->setLightPos(camera->getEye());

	update_proc();

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
	if (player->getAlive())
	{
		player->drawWithUpdate(light.get());
	}
}

void BaseStage::drawFrontSprite()
{
	constexpr ImGuiWindowFlags winFlags =
		// リサイズ不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		// タイトルバー無し
		ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
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
