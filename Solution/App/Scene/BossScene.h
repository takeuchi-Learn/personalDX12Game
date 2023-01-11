/*****************************************************************//**
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
#include "GameObject/Player.h"
#include "3D/Obj/Object3d.h"
#include "3D/Obj/ObjSet.h"
#include "GameObject/BaseEnemy.h"
#include "GameObject/Boss/BossEnemy.h"
#include <Sound/Sound.h>

#include "BaseStage.h"

 /// @brief ボス戦シーンクラス
class BossScene :
	public BaseStage
{
private:
	bool playerUpTurn = false;

	std::unique_ptr<ObjModel> bossModel;
	std::unique_ptr<BossEnemy> boss;
	uint16_t bossHpMax{};

	std::unique_ptr<ObjModel> bossPartsModel;
	std::vector<std::unique_ptr<BaseEnemy>> bossParts;

	// スプライト
	inline static constexpr DirectX::XMFLOAT2 hpGrSizeMax = DirectX::XMFLOAT2(WinAPI::window_width * 0.75f,
																			  WinAPI::window_height / 40.f);
	std::unique_ptr<Sprite> bossHpGr;

	const float playerHpBarWidMax;
	std::unique_ptr<Sprite> playerHpBar;
	std::unique_ptr<Sprite> playerHpBarEdge;

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



#pragma region 音

	std::unique_ptr<Sound> killSe;
	std::unique_ptr<Sound> bossDamageSe;
	std::unique_ptr<Sound> bgm;

#pragma endregion 音

private:
	void createParticle(const DirectX::XMFLOAT3& pos,
						const uint16_t particleNum = 10U,
						const float startScale = 1.f,
						const float vel = 5.f,
						const DirectX::XMFLOAT3& startCol = { 1.f, 1.f, 0.25f },
						const DirectX::XMFLOAT3& endCol = { 1.f, 0.f, 1.f });

	void startRgbShift();
	void updateRgbShift();

	/// @return 照準内に敵がいるかどうか
	bool addShotTarget(const std::forward_list<BaseEnemy*>& enemy,
					   const DirectX::XMFLOAT2& aim2DPosMin,
					   const DirectX::XMFLOAT2& aim2DPosMax);

	void movePlayer() override;

#pragma region updateの中身

	// update_何とか関数を格納する
	void update_start() override;
	void update_appearBoss();
	void update_play() override;
	void update_killBoss();
	void update_end() override;

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
		float startPlayerHpGrScale;
		float endPlayerHpGrScale;
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

	void additionalDrawObj3d() override;

#pragma region 初期化

	void initSprite();
	void initGameObj();
	void initPlayer();
	void initEnemy();
	void initBoss();

#pragma endregion 初期化

#pragma region その他

	inline uint32_t calcBossHp() const
	{
		uint32_t bossHp = 0ui32;
		for (auto& i : bossParts)
		{
			if (i->getAlive())
			{
				bossHp += (uint32_t)i->getHp();
			}
		}
		return bossHp;
	}

#pragma endregion その他

public:
	BossScene();
	~BossScene();

	void start() override;
	void drawFrontSprite() override;
};
