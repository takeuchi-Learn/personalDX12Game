#pragma once
#include "BaseEnemy.h"
class BossEnemy :
	public BaseEnemy
{
public:
	using BaseEnemy::BaseEnemy;

	void phase_approach(GameObj* targetObj);
	inline void changePhase_approach(GameObj* targetObj) { setPhase(std::bind(&BossEnemy::phase_approach, this, targetObj)); }
};
