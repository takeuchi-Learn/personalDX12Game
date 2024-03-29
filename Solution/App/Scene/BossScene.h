﻿/*****************************************************************//**
 * \file   BossScene.h
 * \brief  ボス戦のシーン
 *********************************************************************/

#pragma once
#include "System/GameScene.h"

#include <functional>
#include <memory>
#include <forward_list>

#include "Input/Input.h"
#include "Util/Timer.h"
#include "Camera/CameraObj.h"
#include "2D/Sprite.h"
#include "GameObject/Player/Player.h"
#include <GameObject/Player/Reticle.h>
#include "3D/Obj/Object3d.h"
#include "3D/ParticleMgr.h"
#include "GameObject/BaseEnemy.h"
#include "GameObject/Boss/BossEnemy.h"
#include <CollisionMgr.h>
#include <Sound/Sound.h>

 /// @brief ボス戦シーンクラス
class BossScene :
	public GameScene
{
private:

#pragma region シーン内共通

	DX12Base* dxBase = nullptr;
	Input* input = nullptr;

	std::unique_ptr<CameraObj> camera;

	std::unique_ptr<Light> light;

	std::unique_ptr<Timer> timer;

#pragma endregion シーン内共通

#pragma region 自機

	bool playerUpTurn = false;

#pragma endregion 自機

#pragma region ボス

	std::unique_ptr<ObjModel> bossBulModel;
	std::unique_ptr<BossEnemy> boss;
	uint16_t bossHpMax{};

	std::unique_ptr<ObjModel> bossPartsModel;
	std::vector<std::shared_ptr<BaseEnemy>> bossParts;

#pragma endregion ボス

#pragma region 3Dオブジェクト

	// 背景のパイプライン
	size_t backPipelineSet;

	// 背景と地面
	std::unique_ptr<Object3d> backObj;
	std::unique_ptr<ObjModel> backModel;
	std::unique_ptr<Object3d> groundObj;
	std::unique_ptr<ObjModel> groundModel;

	std::unique_ptr<GameObj> playerParent;

	// 自機
	std::unique_ptr<Player> player;
	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<ObjModel> playerBulModel;
	uint16_t playerHpMax;
	CollisionMgr::ColliderSet playerColliderSet{};

	// 攻撃可能な敵リスト
	std::forward_list<std::weak_ptr<BaseEnemy>> attackableEnemy;

	// パーティクル
	std::unique_ptr<ParticleMgr> particleMgr;

#pragma endregion 3Dオブジェクト

#pragma region スプライト

	std::unique_ptr<SpriteBase> spBase;

	UINT aimGrNum{};
	std::unique_ptr<Sprite> cursorGr;

	std::forward_list<Reticle> reticle;

	struct HpBar
	{
		float backNowRaito = 0.f;
		float frontNowRaito = 0.f;
	};

	inline static constexpr float playerHpBarWidMax = WinAPI::window_width * 0.25f;
	HpBar playerHpBar;

	inline static constexpr float bossHpBarWidMax = WinAPI::window_width * 0.75f;
	HpBar bossHpBar;

#pragma endregion スプライト

#pragma region ImGui定数

	// 最初のウインドウの位置を指定
	static constexpr DirectX::XMFLOAT2 fstWinPos =
		DirectX::XMFLOAT2((float)WinAPI::window_width * 0.02f,
						  (float)WinAPI::window_height * 0.02f);

#pragma endregion

#pragma region 演出

	// RGBずらし
	static const Timer::timeType rgbShiftTimeMax = Timer::oneSec / 2;
	Timer::timeType nowRgbShiftTime = 0;
	Timer::timeType startRgbShiftTime = 0;
	bool rgbShiftFlag = false;

	const static Timer::timeType sceneChangeTime = Timer::oneSec * 3;
	DirectX::XMFLOAT3 sceneChangeStartPos{};
	DirectX::XMFLOAT3 sceneChangeEndPos{};
	DirectX::XMFLOAT3 sceneChangeStartRota{};
	DirectX::XMFLOAT3 sceneChangeEndRota{};
	float sceneChangeStartCamLen;
	float sceneChangeEndCamLen;

#pragma endregion 演出

#pragma region 音

	std::unique_ptr<Sound> killSe;
	std::unique_ptr<Sound> bossDamageSe;
	std::unique_ptr<Sound> bgm;

#pragma endregion 音

private:
	void startRgbShift();
	void updateRgbShift();

	/// @return 照準内に敵がいるかどうか
	bool addShotTarget(const std::forward_list<std::weak_ptr<BaseEnemy>>& enemy,
					   const DirectX::XMFLOAT2& aim2DPos);

	void movePlayer();

	// 照準の位置を移動
	void moveAim2DPos();

	void rotaBackObj();

#pragma region updateの中身

	// update_何とか関数を格納する
	std::function<void()> update_proc;
	void update_start();
	void update_appearBoss();
	void update_play();
	void update_killBoss();
	template<class NextScene>
	void update_end();

#pragma endregion updateの中身

#pragma region ボス登場演出

	struct AppearBossData
	{
		float appearBossTime;
		float startCamLen;
		float endCamLen;
		float startBossHpGrScale;
		float endBossHpGrScale;
		float startCamAngle;
		float endCamAngle;
	};
	std::unique_ptr<AppearBossData> appearBossData;

	struct CameraParam
	{
		GameObj* parentObj;
		DirectX::XMFLOAT3 angleRad;
		float eye2TargetLen;
	};
	std::unique_ptr<CameraParam> camParam;

	void startAppearBoss();
	void endAppearBoss();

#pragma endregion ボス登場演出

#pragma region ボス死亡演出

	struct KillBossData
	{
		float startBossScale;
		float endBossScale;
	};
	std::unique_ptr<KillBossData> killBossData;

	void startKillBoss();

	void endKillBoss();

#pragma endregion ボス死亡演出

#pragma region 初期化

	void initSprite();
	void initGameObj();
	void initPlayer();
	void initEnemy();
	void initBoss();
	void initBackObj();

#pragma endregion 初期化

#pragma region その他

	uint32_t calcBossHp() const;

#pragma endregion その他

public:
	BossScene();
	~BossScene();

	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
