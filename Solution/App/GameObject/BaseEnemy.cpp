#include "BaseEnemy.h"

BaseEnemy::BaseEnemy(Camera* camera,
					 ObjModel* model,
					 const DirectX::XMFLOAT3& pos,
					 uint16_t hp) :
	GameObj(camera, model, pos),
	camera(camera),
	hp(hp),
	phase([] {})
{
}

bool BaseEnemy::damage(uint16_t damegeNum, bool killFlag)
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

void BaseEnemy::additionalUpdate()
{
	if (alive)
	{
		phase();
	}
	afterUpdate();
}