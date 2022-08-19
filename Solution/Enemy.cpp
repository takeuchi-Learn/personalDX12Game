#include "Enemy.h"

Enemy::Enemy(Camera *camera,
			 ObjModel *model,
			 const DirectX::XMFLOAT3 &pos)
	:GameObj(camera, model, pos),
	phase(std::bind(&Enemy::phase_Approach, this)) {
}

#pragma region phase

void Enemy::phase_Approach() {
	obj->position.x += vel.x;
	obj->position.y += vel.y;
	obj->position.z += vel.z;

	if (obj->position.z < 0.f) {
		vel = DirectX::XMFLOAT3(-1, 1, 0);
		phase = std::bind(&Enemy::phase_Leave, this);
	}
}

void Enemy::phase_Leave() {
	obj->position.x += vel.x;
	obj->position.y += vel.y;
	obj->position.z += vel.z;

	if (std::abs(obj->position.x) > 50.f &&
		std::abs(obj->position.y) > 50.f) {
		alive = false;
	}
}

#pragma endregion phase

void Enemy::update(Light *light) {
	if (alive) {
		phase();
	}
}
