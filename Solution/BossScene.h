#pragma once
#include "GameScene.h"
#include "Light.h"
#include "CameraObj.h"
#include "ObjModel.h"
#include "Player.h"
#include "Input.h"

#include <functional>
#include <memory>

class BossScene :
	public GameScene
{
private:
	Input* input = nullptr;

	// 背景のパイプライン
	Object3d::PipelineSet backPipelineSet;

	// 背景と地面
	std::unique_ptr<ObjSet> back;
	std::unique_ptr<ObjSet> ground;

	std::unique_ptr<CameraObj> camera;
	std::unique_ptr<Light> light;

	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<ObjModel> playerBulModel;
	std::unique_ptr<Player> player;

	// update_何とか関数を格納する
	std::function<void()> update_proc;
	void update_start();
	void update_play();
	void update_end();

public:
	BossScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};

