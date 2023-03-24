#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

XMVECTOR Player::getLookVec(float len)
{
	return XMVector3Rotate(XMVectorSet(0, 0, len, 1),
						   XMQuaternionRotationRollPitchYaw(obj->rotation.x,
															obj->rotation.y,
															obj->rotation.z));
}

bool Player::damage(uint16_t damegeNum, bool killFlag)
{
	if (damegeNum >= hp)
	{
		hp = 0u;
		if (killFlag) { kill(); }
		return true;
	}

	hp -= damegeNum;
	return false;
}

bool Player::shot(Camera* camera,
				  ObjModel* model,
				  float speed,
				  float bulScale)
{
	bool ret = false;
	for (const auto& i : shotTargetObjPt)
	{
		PlayerBullet& pb = bul.emplace_front(camera, model, obj->position);
		pb.setScale(bulScale);
		pb.setCol(XMFLOAT4(1.f, 0.25f, 0.25f, 1.f));
		pb.setPos(this->calcWorldPos());
		pb.setLife(bulLife);
		pb.setSpeed(speed);
		XMFLOAT3 tmp{};
		XMStoreFloat3(&tmp, XMVector3Transform(XMVectorSet(0, 0, speed, 1), obj->getMatRota()));
		pb.setVel(tmp);

		if (!i.expired() && i.lock()->getAlive())
		{
			pb.setTargetObjPt(i);
			ret = true;
		}
	}

	deleteShotTarget();
	return ret;
}

void Player::additionalUpdate()
{
	// 死んだ弾は消す
	bul.remove_if([](PlayerBullet& i) { return !i.getAlive(); });
}

void Player::additionalDraw(Light* light)
{
	for (PlayerBullet& i : bul)
	{
		i.drawWithUpdate(light);
	}
}