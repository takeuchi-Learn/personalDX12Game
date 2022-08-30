#include "GameObj.h"

GameObj::GameObj(Camera *camera,
				 ObjModel *model,
				 const DirectX::XMFLOAT3 &pos)
	: obj(std::make_unique<Object3d>(camera,
									 model,
									 0U)) {
	setPos(pos);
}

void GameObj::update() {
	additionalUpdate();
	if (alive) {
		obj->update();
	}
}

void GameObj::draw(Light *light) {
	if (alive) {
		obj->draw(DX12Base::getInstance(), light);
	}
	additionalDraw(light);
}

void GameObj::drawWithUpdate(Light *light) {
	update();

	draw(light);

	additionalDraw(light);
}
