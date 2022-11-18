#pragma once
#include "BaseEnemy.h"
class BossEnemy :
	public BaseEnemy
{

	float moveSpeed = 2.f;

	DirectX::XMVECTOR calcVelVec(GameObj* targetObj);

	void moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec);

public:
	using BaseEnemy::BaseEnemy;

	void phase_approach(GameObj* targetObj);
	inline void changePhase_approach(GameObj* targetObj) { setPhase(std::bind(&BossEnemy::phase_approach, this, targetObj)); }
	
	void phase_leave(GameObj* targetObj);
};
