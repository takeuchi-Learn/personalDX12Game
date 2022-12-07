#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

Player::Player(Camera* camera,
			   ObjModel* model,
			   const DirectX::XMFLOAT3& pos,
			   uint16_t hp)
	: GameObj(camera, model, pos),
	hp(hp)
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
	if (!shotTargetObjPt)
	{
		PlayerBullet& i = bul.emplace_front(camera, model, obj->position);
		i.setScale(bulScale);
		i.setParent(obj->parent);
		i.setVel(XMFLOAT3(0, 0, speed));
	} else
	{
		PlayerBullet& i = bul.emplace_front(camera, model, obj->position);
		i.setScale(bulScale);
		i.setParent(obj->parent);

		const XMFLOAT3 player2ShotTaregt{
			shotTargetObjPt->position.x - obj->position.x,
			shotTargetObjPt->position.y - obj->position.y,
			shotTargetObjPt->position.z - obj->position.z,
		};

		// 照準のある方向へ、速さvelで飛んでいく
		XMFLOAT3 vel{};
		XMStoreFloat3(&vel, speed * XMVector3Normalize(XMVectorSet(player2ShotTaregt.x,
																   player2ShotTaregt.y,
																   player2ShotTaregt.z,
																   1)));

		i.setVel(vel);
		const XMFLOAT2 rota = GameObj::calcRotationSyncVelDeg(vel);
		i.setRotation(XMFLOAT3(rota.x, rota.y, i.getRotation().z));
	}
}

void Player::additionalUpdate()
{
	// 死んだ弾は消す
	bul.remove_if([](PlayerBullet& i) { return !i.getAlive(); });
}

void Player::additionalDraw(Light* light)
{
	for (auto& i : bul)
	{
		i.drawWithUpdate(light);
	}
}