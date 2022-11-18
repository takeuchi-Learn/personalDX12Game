#pragma once
#include "BaseEnemy.h"
class BossEnemy :
	public BaseEnemy
{
	// 攻撃対象へのポインタ
	GameObj* targetObj = nullptr;

	// 移動速度
	float moveSpeed = 2.f;

	DirectX::XMVECTOR calcVelVec();

	void moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec);

public:
	using BaseEnemy::BaseEnemy;

	inline void setTargetObj(GameObj* obj) { targetObj = obj; }
	inline GameObj* getTargetObj() { return targetObj; }

	void phase_approach();
	inline void changePhase_approach() { setPhase(std::bind(&BossEnemy::phase_approach, this)); }

	void phase_leave();
};
