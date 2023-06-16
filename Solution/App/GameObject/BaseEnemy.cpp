#include "BaseEnemy.h"

BaseEnemy::BaseEnemy(Camera* camera,
					 ObjModel* model,
					 const DirectX::XMFLOAT3& pos,
					 uint16_t hp) :
	GameObj(camera, model, pos),
	camera(camera),
	hp(hp),
	phase([] {})
{}

void BaseEnemy::additionalUpdate()
{
	beforeUpdate();
	if (alive)
	{
		phase();

		movePos(vel);
	}
	afterUpdate();
}