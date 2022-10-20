#include "GameObj.h"

using namespace DirectX;

DirectX::XMFLOAT3 GameObj::calcWorldPos() const
{
	XMFLOAT3 worldPos = obj->position;

	for (Object3d* parent = obj->parent;
		 parent != nullptr;
		 parent = parent->parent)
	{
		worldPos.x += parent->position.x;
		worldPos.y += parent->position.y;
		worldPos.z += parent->position.z;
	}

	return worldPos;
}

GameObj::GameObj(Camera* camera,
				 ObjModel* model,
				 const DirectX::XMFLOAT3& pos)
	: obj(std::make_unique<Object3d>(camera,
									 model,
									 0U))
{
	setPos(pos);
}

void GameObj::update()
{
	additionalUpdate();
	if (alive)
	{
		obj->update();
	}
}

void GameObj::draw(Light* light)
{
	if (alive)
	{
		obj->draw(DX12Base::getInstance(), light);
	}
	additionalDraw(light);
}

void GameObj::drawWithUpdate(Light* light)
{
	update();

	draw(light);

	additionalDraw(light);
}