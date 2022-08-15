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
	Object3d::PipelineSet object3dPipelineSet;
	Object3d::PipelineSet backPipelineSet;

	std::unique_ptr<ObjSet> back;

	std::unique_ptr<Player> player;

	std::unique_ptr<ObjModel> playerBulModel;



	static const Time::timeType sceneChangeTime;

	Time::timeType startSceneChangeTime{};



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

