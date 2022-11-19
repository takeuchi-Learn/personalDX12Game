#pragma once
#include "BaseEnemy.h"
#include <forward_list>
class BossEnemy :
	public BaseEnemy
{
	// 攻撃対象へのポインタ
	GameObj* targetObj = nullptr;

	// 移動速度
	float moveSpeed = 2.f;

	std::forward_list<std::unique_ptr<BaseEnemy>> smallEnemy;

	DirectX::XMVECTOR calcVelVec(GameObj* me, bool moveYFlag = false);

	void moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec);

	void afterUpdate() override;
	void additionalDraw(Light* light) override;

public:
	using BaseEnemy::BaseEnemy;

	size_t calcSmallEnemyNum() const { return std::distance(smallEnemy.begin(), smallEnemy.end()); }

	inline void setTargetObj(GameObj* obj) { targetObj = obj; }
	inline GameObj* getTargetObj() { return targetObj; }

	void addSmallEnemy(ObjModel* model);

	void phase_approach();
	inline void changePhase_approach() { setPhase(std::bind(&BossEnemy::phase_approach, this)); }

	void phase_leave();
};
