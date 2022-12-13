/*****************************************************************//**
 * \file   BaseStage.h
 * \brief  ステージ基底クラス
 *********************************************************************/

#pragma once
#include "System/GameScene.h"

#include "Input/Input.h"
#include "System/DX12Base.h"
#include "Camera/CameraObj.h"
#include "GameObject/Player.h"
#include "Util/Timer.h"
#include "3D/Obj/ObjSet.h"
#include "3D/ParticleMgr.h"
#include "GameObject/BaseEnemy.h"
#include "2D/SpriteBase.h"
#include "2D/Sprite.h"

#include <memory>
#include <functional>
#include <forward_list>

 /// @brief ステージ基底クラス
class BaseStage
	: public GameScene
{
public:
	// std::stringの2次元配列(vector)
	using CSVType = std::vector<std::vector<std::string>>;

protected:
	DX12Base* dxBase = nullptr;
	Input* input = nullptr;

	std::unique_ptr<CameraObj> camera;

	std::unique_ptr<Light> light;

	std::unique_ptr<Timer> timer;

#pragma region 3Dオブジェクト

	// 背景のパイプライン
	Object3d::PipelineSet backPipelineSet;

	// 背景と地面
	std::unique_ptr<ObjSet> back;
	std::unique_ptr<ObjSet> ground;

	// 自機
	std::unique_ptr<Player> player;
	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<ObjModel> playerBulModel;
	uint16_t playerHpMax;

	// 攻撃可能な敵リスト
	std::forward_list<BaseEnemy*> attackableEnemy;

	// パーティクル
	std::unique_ptr<ParticleMgr> particleMgr;

#pragma endregion 3Dオブジェクト

	// スプライト
	std::unique_ptr<SpriteBase> spBase;
	std::unique_ptr<Sprite> aim2D;

	// 敵発生スクリプトのCSVデータ
	CSVType csvData;

#pragma region ImGui定数

	static constexpr ImGuiWindowFlags winFlags =
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
	static constexpr DirectX::XMFLOAT2 fstWinPos =
		DirectX::XMFLOAT2((float)WinAPI::window_width * 0.02f,
						  (float)WinAPI::window_height * 0.02f);

#pragma endregion

#pragma region updateの中身

	// update_何とか関数を格納する
	std::function<void()> update_proc;
	virtual void update_start();
	virtual void update_play();
	virtual void update_end();

	virtual void movePlayer();

#pragma endregion updateの中身

	virtual void additionalDrawObj3d() {};

	void initPlayer();
	void initBackObj();
	void initSprite();

public:

	CSVType loadCsv(const std::string& csvFilePath,
					bool commentFlag = true,
					char divChar = ',',
					const std::string& commentStartStr = "//");

public:
	BaseStage();

	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	~BaseStage();
};
