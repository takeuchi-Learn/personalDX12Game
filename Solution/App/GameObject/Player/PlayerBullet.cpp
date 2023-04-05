#include "PlayerBullet.h"
#include <Util/RandomNum.h>

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

	if (alive)
	{
		if (!particle.expired())
		{
			particle.lock()->add(Timer::oneSec,
								 this->calcWorldPos(),
								 XMFLOAT3(0, RandomNum::getRandNormallyf(0.f, 0.1f), 0.f),
								 XMFLOAT3(),
								 4.f, 0.f,
								 0.f, 0.f,
								 XMFLOAT3(1, 1, 1),
								 XMFLOAT3(0, 0, 0));
		}

		// 追従対象がいればそちらへ進む
		if (!targetObjPt.expired())
		{
			auto target = targetObjPt.lock();
			if (target->getAlive())
			{
				const XMFLOAT3 prevel = vel;

				// 自機から攻撃対象へ向かう方向のベクトル
				vel = GameObj::calcVel(target->calcWorldPos(), this->calcWorldPos(), speed);

				// 親がいればその回転を反映させる
				if (const BaseObj* parent = getParent())
				{
					XMVECTOR velVec = XMVector3Rotate(XMLoadFloat3(&vel),
													  XMQuaternionRotationMatrix(parent->getMatRota()));
					XMStoreFloat3(&vel, velVec);
				}

				vel.x = std::lerp(prevel.x, vel.x, homingRaito);
				vel.y = std::lerp(prevel.y, vel.y, homingRaito);
				vel.z = std::lerp(prevel.z, vel.z, homingRaito);

				// 進行方向を向く
				XMFLOAT2 rota = GameObj::calcRotationSyncVelDeg(vel);
				setRotation(XMFLOAT3(rota.x, rota.y, obj->rotation.z));
			}
		}

		// 移動
		obj->position.x += vel.x;
		obj->position.y += vel.y;
		obj->position.z += vel.z;
	}
}