#include "PlayerBullet.h"

void PlayerBullet::update(Light *light) {
	// 寿命が来たら死ぬ
	++age;
	if (age > life) {
		alive = false;
	}

	if (alive) {
		obj->position.x += vel.x;
		obj->position.y += vel.y;
		obj->position.z += vel.z;
	}
}
