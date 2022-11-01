#include "BaseEnemy.h"

BaseEnemy::BaseEnemy(Camera* camera,
					 ObjModel* model,
					 const DirectX::XMFLOAT3& pos) :
	GameObj(camera, model, pos),
	phase([] {})
{
}

void BaseEnemy::additionalUpdate()
{
	if (alive)
	{
		phase();
	}
	afterUpdate();
}
