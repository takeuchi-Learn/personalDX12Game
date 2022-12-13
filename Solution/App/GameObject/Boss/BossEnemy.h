/*****************************************************************//**
 * \file   BossEnemy.h
 * \brief  ボス敵クラス
 *********************************************************************/

#pragma once
#include "GameObject/BaseEnemy.h"
#include <forward_list>

/// @brief ボス敵クラス
class BossEnemy :
	public BaseEnemy
{
	// 攻撃対象へのポインタ
	GameObj* targetObj = nullptr;

	// 移動速度
	float moveSpeed = 2.f;

	DirectX::XMVECTOR calcVelVec(GameObj* me, bool moveYFlag = false);

	void moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec);

	void afterUpdate() override;
	void additionalDraw(Light* light) override;

public:
	using BaseEnemy::BaseEnemy;

	inline void setTargetObj(GameObj* obj) { targetObj = obj; }
	inline GameObj* getTargetObj() { return targetObj; }

	void phase_approach();
	inline void changePhase_approach() { setPhase(std::bind(&BossEnemy::phase_approach, this)); }

	void phase_leave();

	void phase_attack();

#pragma region 弾関係

private:
	ObjModel* smallEnemyModel = nullptr;
	std::forward_list<std::unique_ptr<BaseEnemy>> smallEnemy;
	float smallEnemyMoveSpeed = 2.f;

	static inline const uint32_t shotInterval = 60u;
	uint32_t nowShotFrame = 0u;
	static inline const uint32_t shotNumMax = 5u;
	uint32_t shotNum = 0u;

public:
	/// @brief 小さい敵を弾として出す
	void addSmallEnemy();

	/// @brief 弾として出された小さい敵の数を算出
	/// @return 小さい敵の数
	inline size_t calcSmallEnemyNum() const { return std::distance(smallEnemy.begin(), smallEnemy.end()); }

	inline const auto& getSmallEnemyList() const { return smallEnemy; }

	inline ObjModel* getSmallEnemyModel() { return smallEnemyModel; }
	inline void setSmallEnemyModel(ObjModel* model) { smallEnemyModel = model; }

#pragma endregion 弾関係
};
