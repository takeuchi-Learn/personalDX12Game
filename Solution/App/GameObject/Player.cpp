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

void Player::shot(Camera* camera,
				  ObjModel* model,
				  float speed,
				  float bulScale)
{
	PlayerBullet& i = bul.emplace_front(camera, model, obj->position);
	i.setScale(bulScale);
	i.setCol(XMFLOAT4(1.f, 0.25f, 0.25f, 1.f));
	i.setPos(this->calcWorldPos());
	i.setLife(bulLife);
	i.setSpeed(speed);
	XMFLOAT3 tmp{};
	XMStoreFloat3(&tmp, XMVector3Transform(XMVectorSet(0, 0, speed, 1), obj->getMatRota()));
	i.setVel(tmp);

	if (!shotTargetObjPt.expired())
	{
		i.setTargetObjPt(shotTargetObjPt);
	}
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