#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

Player::Player(Camera* camera,
			   ObjModel* model,
			   const DirectX::XMFLOAT3& pos,
			   uint16_t hp)
	: GameObj(camera, model, pos),
	hp(hp),
	shotTargetObjPt(nullptr)
{
}

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

void Player::moveForward(float moveVel, bool moveYFlag)
{
	// Z方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(0, 0, moveVel, 1), obj->getMatRota());

	// Y方向に移動しないならY成分を消す
	if (!moveYFlag)
	{
		// absがあるのは、大きさのみ指定したいから。
		// absがないと、moveVelがマイナスの場合に
		// マイナス * マイナスでプラスになってしまう
		velVec = XMVectorScale(XMVector3Normalize(XMVectorSetY(velVec, 0.f)),
							   std::abs(moveVel));
	}

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void Player::moveRight(float moveVel, bool moveYFlag)
{
	// X方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(moveVel, 0, 0, 1), obj->getMatRota());

	// Y方向に移動しないならY成分を消す
	if (!moveYFlag)
	{
		// absがあるのは、大きさのみ指定したいから。
		// absがないと、moveVelがマイナスの場合に
		// マイナス * マイナスでプラスになってしまう
		velVec = XMVectorScale(XMVector3Normalize(XMVectorSetY(velVec, 0.f)),
							   std::abs(moveVel));
	}

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void Player::moveUp(float moveVel)
{
	// Y方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(0, moveVel, 0, 1), obj->getMatRota());

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void Player::shot(Camera* camera,
				  ObjModel* model,
				  float speed,
				  float bulScale)
{
	PlayerBullet& i = bul.emplace_front(camera, model, obj->position);
	i.setScale(bulScale);
	i.setCol(XMFLOAT4(1.f, 0.25f, 0.25f, 1.f));
	i.setParent(obj->parent);
	i.setLife(bulLife);
	i.setSpeed(speed);
	XMFLOAT3 tmp{};
	XMStoreFloat3(&tmp, XMVector3Transform(XMVectorSet(0, 0, speed, 1), obj->getMatRota()));
	i.setVel(tmp);

	if (shotTargetObjPt)
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