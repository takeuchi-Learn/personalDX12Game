﻿#include "BossEnemy.h"

BossEnemy::BossEnemy(Camera* camera,
					 ObjModel* model,
					 const DirectX::XMFLOAT3& pos,
					 uint16_t hp) :
	BaseEnemy(camera, model, pos),
	hp(hp)
{
}

bool BossEnemy::damage(uint16_t damegeNum, bool killFlag)
{
	if (damegeNum >= hp)
	{
		hp = 0u;
		if (killFlag) { kill(); }
		return true;
	}

	hp -= damegeNum;
	return false;
}
