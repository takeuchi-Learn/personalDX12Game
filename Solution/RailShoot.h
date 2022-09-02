#pragma once

#include "GameScene.h"
#include "CameraObj.h"
#include <memory>

#include "Player.h"
#include "ObjSet.h"
#include "Light.h"

#include "Time.h"

#include "Input.h"

#include <functional>

#include "DebugText.h"

#include "Enemy.h"

#include "ParticleMgr.h"

class RailShoot
	: public GameScene {

	DX12Base *dxBase = nullptr;
	Input *input = nullptr;


	std::unique_ptr<CameraObj> camera;

	std::unique_ptr<Light> light;

	std::unique_ptr<Time> timer;


	// --------------------
	// スプライト
	// --------------------
	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<DebugText> debugText;

	std::unique_ptr<Sprite> aim2D;

	// --------------------
	// 3Dオブジェクト
	// --------------------
	// 背景のパイプライン
	Object3d::PipelineSet backPipelineSet;

	// 背景と地面
	std::unique_ptr<ObjSet> back;
	std::unique_ptr<ObjSet> ground;

	// 敵
	std::forward_list<std::unique_ptr<Enemy>> enemy;
	std::unique_ptr<ObjModel> enemyModel;
	std::unique_ptr<ObjModel> enemyBulModel;

	// 自機
	std::unique_ptr<Player> player;
	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<ObjModel> playerBulModel;
	UINT playerHp;

	// レールの現在位置を示すオブジェクト
	std::unique_ptr<GameObj> railObj;

	// --------------------
	// パーティクル
	// --------------------
	std::unique_ptr<ParticleMgr> particleMgr;

	// --------------------
	// シーン遷移
	// --------------------
	static const Time::timeType sceneChangeTime;

	Time::timeType startSceneChangeTime{};
	// --------------------
	// スプライン補間
	// --------------------
	std::vector<DirectX::XMVECTOR> splinePoint;
	UINT splineNowFrame = 0u;
	static const UINT splineFrameMax = 120u;
	static const UINT splineIndexDef = 1u;
	UINT splineIndex = splineIndexDef;



	// std::stringの2次元配列(vector)
	using CSVType = std::vector<std::vector<std::string>>;

	// 敵発生スクリプトのCSVデータ
	CSVType csvData;

	CSVType loadCsv(const std::string &csvFilePath,
					bool commentFlag = true,
					char divChar = ',',
					const std::string &commentStartStr = "//");

	struct PopEnemyData {
		UINT popFrame;
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel{ 0,0,-1 };
		PopEnemyData(UINT popFrame,
					 const DirectX::XMFLOAT3 &pos,
					 const DirectX::XMFLOAT3 &vel)
			: popFrame(popFrame), pos(pos), vel(vel) {
		}
	};

	std::forward_list<std::unique_ptr<PopEnemyData>> enemyPopData;
	UINT nowFrame = 0u;



	void createParticle(const DirectX::XMFLOAT3 &pos,
						const UINT particleNum = 10U,
						const float startScale = 1.f,
						const float vel = 5.f);

	void addEnemy(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &vel, float scale = 5.f);

	void changeNextScene();

	// update_何とか関数を格納する
	std::function<void()> update_proc;

	void update_start();
	void update_play();
	void update_end();

public:
	RailShoot();

	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	~RailShoot();

	static DirectX::XMVECTOR splinePosition(const std::vector<DirectX::XMVECTOR> &points,
											size_t startIndex,
											float t);
};

