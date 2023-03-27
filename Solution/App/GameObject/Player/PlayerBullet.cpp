#include "PlayerBullet.h"

using namespace DirectX;

void PlayerBullet::additionalUpdate()
{
	// 対象がおらず、寿命が来たら死ぬ
	if (targetObjPt.expired())
	{
		if (++age > life)
		{
			alive = false;
		}
	}

	if (alive && !targetObjPt.expired())
	{
		auto target = targetObjPt.lock();
		if (target->getAlive())
		{
			// 自機から攻撃対象へ向かう方向のベクトル
			vel = GameObj::calcVel(target->calcWorldPos(), this->calcWorldPos(), speed);

			// 親がいればその回転を反映させる
			if (const BaseObj* parent = getParent())
			{
				XMVECTOR velVec = XMVector3Rotate(XMLoadFloat3(&vel),
												  XMQuaternionRotationMatrix(parent->getMatRota()));
				XMStoreFloat3(&vel, velVec);
			}

			// 進行方向を向く
			XMFLOAT2 rota = GameObj::calcRotationSyncVelDeg(vel);
			setRotation(XMFLOAT3(rota.x, rota.y, obj->rotation.z));
		}

		// 移動
		obj->position.x += vel.x;
		obj->position.y += vel.y;
		obj->position.z += vel.z;
	}
}