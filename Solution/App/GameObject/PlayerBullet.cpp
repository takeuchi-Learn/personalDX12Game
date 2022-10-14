#include "PlayerBullet.h"

void PlayerBullet::additionalUpdate()
{
	// 寿命が来たら死ぬ
	if (++age > life)
	{
		alive = false;
	}

	if (alive)
	{
		obj->position.x += vel.x;
		obj->position.y += vel.y;
		obj->position.z += vel.z;
	}
}