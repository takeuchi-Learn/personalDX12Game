#include "GameObj.h"

using namespace DirectX;

DirectX::XMFLOAT3 GameObj::calcWorldPos() const
{
	return XMFLOAT3(obj->getMatWorld().r[3].m128_f32[0],
					obj->getMatWorld().r[3].m128_f32[1],
					obj->getMatWorld().r[3].m128_f32[2]);
}

GameObj::GameObj(Camera* camera,
				 ObjModel* model,
				 const DirectX::XMFLOAT3& pos)
	: obj(std::make_unique<Object3d>(camera,
									 model))
{
	setPos(pos);
}

GameObj::~GameObj()
{
	obj.reset(nullptr);
}

void GameObj::update()
{
	additionalUpdate();
	obj->update();
}

void GameObj::draw(Light* light)
{
	if (drawFlag)
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