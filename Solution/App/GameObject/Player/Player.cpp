#include "Player.h"
#include <DirectXMath.h>

#include <Util/RandomNum.h>

using namespace DirectX;

XMVECTOR Player::getLookVec(float len)
{
	return XMVector3Rotate(XMVectorSet(0, 0, len, 1),
						   XMQuaternionRotationRollPitchYaw(obj->rotation.x,
															obj->rotation.y,
															obj->rotation.z));
}

Player::Player(Camera* camera,
			   ObjModel* model,
			   const DirectX::XMFLOAT3& pos) :
	GameObj(camera, model, pos),
	bulParticle(std::make_shared<ParticleMgr>())
{
	bulParticle->setCamera(camera);
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

bool Player::shotAll(Camera* camera,
				  ObjModel* model,
				  float speed,
				  float bulScale)
{
	bool ret = false;
	for (const auto& i : shotTargetObjPt)
	{
		PlayerBullet& pb = bul.emplace_front(camera, model, this->calcWorldPos());
		pb.setScale(bulScale);
		pb.setCol(XMFLOAT4(1.f, 0.25f, 0.25f, 1.f));
		pb.setLife(bulLife);
		pb.setSpeed(speed);
		pb.setParticle(bulParticle);
		pb.setHomingRaito(bulHomingRaito);

		if (!i.expired() && i.lock()->getAlive())
		{
			pb.setTargetObjPt(i);
			ret = true;

			XMVECTOR velVec = XMVector3Transform(XMVectorSet(0, 0, speed, 1), obj->getMatRota());
			velVec = XMVector3Rotate(velVec, XMQuaternionRotationRollPitchYaw(RandomNum::getRandf(0.f, XM_2PI), XM_PIDIV2, 0.f));

			XMFLOAT3 velF3{};
			XMStoreFloat3(&velF3, velVec);
			pb.setVel(velF3);
		} else
		{
			XMFLOAT3 velF3{};
			XMStoreFloat3(&velF3, XMVector3Transform(XMVectorSet(0, 0, speed, 1), obj->getMatRota()));
			pb.setVel(velF3);
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