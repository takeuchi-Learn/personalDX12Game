#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

Player::Player(Camera *camera,
			   ObjModel *model,
			   const DirectX::XMFLOAT3 &pos)
	: GameObj(camera, model, pos) {

	shotTargetObjPt = nullptr;
}

XMVECTOR Player::getLookVec(float len) {
	return XMVector3Rotate(XMVectorSet(0, 0, len, 1),
						   XMQuaternionRotationRollPitchYaw(obj->rotation.x,
															obj->rotation.y,
															obj->rotation.z));
}

void Player::moveForward(float moveVel, bool moveYFlag) {
	XMVECTOR forward = getLookVec(moveVel);
	if (!moveYFlag) {
		const float velSign = moveVel < 0 ? -1.f : 1.f;

		forward = XMVectorSet(forward.m128_f32[0] * velSign,
							  0,
							  forward.m128_f32[2] * velSign,
							  forward.m128_f32[3] * velSign);
		forward = XMVectorScale(XMVector3Normalize(forward), moveVel);
	}

	const XMVECTOR posVec = DirectX::XMVectorAdd(XMLoadFloat3(&obj->position),
												 forward);

	XMStoreFloat3(&obj->position, posVec);
}

void Player::moveRight(float moveVel, bool moveYFlag) {
	XMVECTOR right = XMVector3Rotate(XMVectorSet(moveVel, 0, 0, 1),
									 XMQuaternionRotationRollPitchYaw(obj->rotation.x,
																	  obj->rotation.y,
																	  obj->rotation.z));
	if (!moveYFlag) {
		const float velSign = moveVel < 0 ? -1.f : 1.f;

		right = XMVectorSet(right.m128_f32[0] * velSign,
							0,
							right.m128_f32[2] * velSign,
							right.m128_f32[3] * velSign);
		right = XMVectorScale(XMVector3Normalize(right), moveVel);
	}

	const XMVECTOR posVec = DirectX::XMVectorAdd(XMLoadFloat3(&obj->position),
												 right);

	XMStoreFloat3(&obj->position, posVec);
}

void Player::shot(Camera *camera,
				  ObjModel *model,
				  float speed,
				  float bulScale) {
	// C++17から追加した要素の参照が返ってくるようになった
	PlayerBullet &i = bul.emplace_front(camera, model, obj->position);
	i.setScale(bulScale);

	if (shotTargetObjPt == nullptr) {
		i.setVel(XMFLOAT3(0, 0, speed));
	} else {
		const XMFLOAT3 player2ShotTaregt{
			shotTargetObjPt->position.x - obj->position.x,
			shotTargetObjPt->position.y - obj->position.y,
			shotTargetObjPt->position.z - obj->position.z,
		};

		// 照準のある方向へ、速さvelで飛んでいく
		XMFLOAT3 vel{};
		XMStoreFloat3(&vel, XMVector3Normalize(XMVectorSet(player2ShotTaregt.x,
														   player2ShotTaregt.y,
														   player2ShotTaregt.z,
														   1)));
		vel.x *= speed;
		vel.y *= speed;
		vel.z *= speed;

		i.setVel(vel);
	}
}

void Player::additionalUpdate() {
	// 死んだ弾は消す
	bul.remove_if([](PlayerBullet &i) {return !i.getAlive(); });
}

void Player::additionalDraw(Light *light) {
	for (auto &i : bul) {
		i.drawWithUpdate(light);
	}
}