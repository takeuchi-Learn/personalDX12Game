#pragma once

#include "GameScene.h"
#include "Camera.h"
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


	std::unique_ptr<Camera> camera;

	std::unique_ptr<Light> light;

	std::unique_ptr<Time> timer;



	// スプライト
	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<DebugText> debugText;



	// 3Dオブジェクト用パイプライン
	Object3d::PipelineSet backPipelineSet;

	std::unique_ptr<ObjSet> back;

	std::vector<std::unique_ptr<Enemy>> enemy;
	std::unique_ptr<ObjModel> enemyModel;
	std::unique_ptr<ObjModel> enemyBulModel;

	std::unique_ptr<Player> player;
	std::unique_ptr<ObjModel> playerModel;

	std::unique_ptr<ObjModel> playerBulModel;


	std::unique_ptr<ParticleMgr> particleMgr;



	static const Time::timeType sceneChangeTime;

	Time::timeType startSceneChangeTime{};



	void createParticle(const DirectX::XMFLOAT3 &pos,
						const UINT particleNum = 10U,
						const float startScale = 1.f,
						const float vel = 5.f);

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

};

