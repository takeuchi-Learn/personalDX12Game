#include "PlayerBullet.h"

PlayerBullet::PlayerBullet(Object3d *obj,
						   const DirectX::XMFLOAT3 &pos)
	: obj(obj),
	pos(pos) {
	obj->position = pos;
}

PlayerBullet::PlayerBullet(Camera *camera,
						   ObjModel *model,
						   const DirectX::XMFLOAT3 &pos)
	:needToDeleteObj(true),
	obj(new Object3d(DX12Base::getInstance()->getDev(),
					 camera,
					 model,
					 0U)) {
	obj->position = pos;
	obj->parent = nullptr;
}

PlayerBullet::~PlayerBullet() {
	if (needToDeleteObj) {
		delete obj;
		obj = nullptr;
	}
}

void PlayerBullet::drawWithUpdate(Light *light) {
	// 寿命が来たら死ぬ
	++age;
	if (age > life) {
		alive = false;
	}

	// 弾が生きていたら更新描画をする
	if (alive) {

		if (posDirty) {
			obj->position = pos;
			posDirty = false;
		} else {
			pos.x += vel.x;
			pos.y += vel.y;
			pos.z += vel.z;
			obj->position = pos;
		}
		obj->drawWithUpdate(DX12Base::getInstance(), light);
	}
}
