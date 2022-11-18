#pragma once
#include "../Engine/System/GameScene.h"

#include <functional>
#include <memory>
#include <forward_list>

#include "../Engine/Input/Input.h"
#include "../Engine/Util/Timer.h"
#include "../Engine/Camera/CameraObj.h"
#include "../Engine/2D/Sprite.h"
#include "../GameObject/Player.h"
#include "../Engine/3D/Obj/Object3d.h"
#include "../Engine/3D/Obj/ObjSet.h"
#include "../GameObject/BaseEnemy.h"
#include "../GameObject/BossEnemy.h"

#include "BaseStage.h"

class BossScene :
	public BaseStage
{
private:
	bool playerUpTurn = false;

	std::unique_ptr<ObjModel> bossModel;
	std::unique_ptr<BossEnemy> boss;
	uint16_t bossHpMax;

	std::unique_ptr<ObjModel> smallEnemyModel;
	std::vector<std::unique_ptr<BaseEnemy>> smallEnemy;
	uint16_t smallEnemyHpMax;

	uint16_t playerHpMax;

	// スプライト
	inline static constexpr DirectX::XMFLOAT2 hpGrSizeMax = DirectX::XMFLOAT2(WinAPI::window_width * 0.75f,
																			  WinAPI::window_height / 40.f);
	std::unique_ptr<Sprite> bossHpGr;

	// RGBずらし
	static const Timer::timeType rgbShiftTimeMax = Timer::oneSec / 2;
	Timer::timeType nowRgbShiftTime = 0;
	Timer::timeType startRgbShiftTime = 0;
	bool rgbShiftFlag = false;

	void startRgbShift();
	void updateRgbShift();

	void movePlayer() override;

	// update_何とか関数を格納する
	void update_start() override;
	void update_play() override;
	void update_end() override;

public:
	BossScene();

	void start() override;
	void drawFrontSprite() override;
};
