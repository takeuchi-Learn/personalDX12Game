#include "Enemy.h"

void Enemy::update(Light *light) {
	if (alive) {
		obj->position.x += vel.x;
		obj->position.y += vel.y;
		obj->position.z += vel.z;
	}
}
