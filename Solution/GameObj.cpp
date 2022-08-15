#include "GameObj.h"

GameObj::GameObj(Camera *camera,
			 ObjModel *model,
			 const DirectX::XMFLOAT3 &pos)
	: obj(std::make_unique<Object3d>(DX12Base::getInstance()->getDev(),
									 camera,
									 model,
									 0U)) {
	setPos(pos);
}

void GameObj::drawWithUpdate(Light *light) {
	update(light);

	if (alive) {
		obj->drawWithUpdate(DX12Base::getInstance(), light);
	}
}
