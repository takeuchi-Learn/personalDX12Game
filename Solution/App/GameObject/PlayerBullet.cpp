#include "PlayerBullet.h"

using namespace DirectX;

void PlayerBullet::additionalUpdate()
{
	// 寿命が来たら死ぬ
	if (++age > life)
	{
		alive = false;
	}

	if (alive)
	{
		if (targetObjPt && targetObjPt->getAlive())
		{
			const float speed = std::sqrt(vel.x * vel.x +
										  vel.y * vel.y +
										  vel.z * vel.z);

			vel = GameObj::calcVel(targetObjPt->calcWorldPos(), this->calcWorldPos(), speed);

			if (const Object3d* parent = getParent())
			{
				XMVECTOR velVec = XMVector3Rotate(XMLoadFloat3(&vel),
												  XMQuaternionRotationMatrix(parent->getMatRota()));
				XMStoreFloat3(&vel, velVec);
			}

			XMFLOAT2 rota = GameObj::calcRotationSyncVelDeg(vel);
			setRotation(XMFLOAT3(rota.x, rota.y, obj->rotation.z));
		}

		obj->position.x += vel.x;
		obj->position.y += vel.y;
		obj->position.z += vel.z;
	}
}