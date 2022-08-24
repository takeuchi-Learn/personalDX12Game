#include "GameObj.h"

GameObj::GameObj(Camera *camera,
				 ObjModel *model,
				 const DirectX::XMFLOAT3 &pos)
	: obj(std::make_unique<Object3d>(camera,
									 model,
									 0U)) {
	setPos(pos);
}

void GameObj::drawWithUpdate(Light *light) {
	update();

	if (alive) {
		obj->drawWithUpdate(DX12Base::getInstance(), light);
	}

	additionalDraw(light);
}
