#pragma once
#include "GameScene.h"
#include "Light.h"
#include "CameraObj.h"
#include "ObjModel.h"
#include "Player.h"
#include "Input.h"
#include "Time.h"

#include "Sprite.h"
#include "SpriteBase.h"

#include <functional>
#include <memory>

class BossScene :
	public GameScene
{
private:
	Input* input = nullptr;

	std::unique_ptr<Time> timer;

	// 背景のパイプライン
	Object3d::PipelineSet backPipelineSet;

	// 背景と地面
	std::unique_ptr<ObjSet> back;
	std::unique_ptr<ObjSet> ground;

	// カメラとライト
	std::unique_ptr<CameraObj> camera;
	std::unique_ptr<Light> light;

	// ゲームオブジェクト
	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<ObjModel> playerBulModel;
	std::unique_ptr<Player> player;
	bool playerUpTurn = false;

	// スプライト
	std::unique_ptr<SpriteBase> spBase;
	std::unique_ptr <Sprite> aim2D;

	// RGBずらし
	static const Time::timeType rgbShiftTimeMax = Time::oneSec / 2;
	Time::timeType nowRgbShiftTime = 0;
	Time::timeType startRgbShiftTime = 0;
	bool rgbShiftFlag = false;

	void startRgbShift();
	void updateRgbShift();

	// update_何とか関数を格納する
	std::function<void()> update_proc;
	void update_start();
	void update_play();
	void update_end();

public:
	BossScene();

	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};

