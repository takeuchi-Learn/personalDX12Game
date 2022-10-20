#pragma once
#include "../Engine/System/GameScene.h"

#include "../Engine/Input/Input.h"
#include "../Engine/System/DX12Base.h"
#include "../Engine/Camera/CameraObj.h"
#include "../GameObject/Player.h"
#include "../Engine/Util/Timer.h"
#include "../Engine/3D/Obj/ObjSet.h"
#include "../Engine/3D/ParticleMgr.h"

#include <memory>
#include <functional>

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
	UINT playerHpMax;
	UINT playerHp;

	// パーティクル
	std::unique_ptr<ParticleMgr> particleMgr;

#pragma endregion 3Dオブジェクト


	// 敵発生スクリプトのCSVデータ
	CSVType csvData;

	// update_何とか関数を格納する
	std::function<void()> update_proc;
	virtual void update_start();
	virtual void update_play();
	virtual void update_end();

	virtual void additionalUpdate_play() {};

	virtual void movePlayer();

protected:
	bool playerDamage(UINT damageNum);

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

