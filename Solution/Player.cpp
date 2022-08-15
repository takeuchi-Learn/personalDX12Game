#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

XMVECTOR Player::getLookVec(float len) {
	return XMVector3Rotate(XMVectorSet(0, 0, len, 1),
						   XMQuaternionRotationRollPitchYaw(obj->getRotation().x,
															obj->getRotation().y,
															obj->getRotation().z));
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

	const XMVECTOR posVec = DirectX::XMVectorAdd(XMLoadFloat3(&obj->getPos()),
												 forward);

	XMFLOAT3 pos{};
	XMStoreFloat3(&pos, posVec);
	obj->setPos(pos);
}

void Player::moveRight(float moveVel, bool moveYFlag) {
	XMVECTOR right = XMVector3Rotate(XMVectorSet(moveVel, 0, 0, 1),
									 XMQuaternionRotationRollPitchYaw(obj->getRotation().x,
																	  obj->getRotation().y,
																	  obj->getRotation().z));
	if (!moveYFlag) {
		const float velSign = moveVel < 0 ? -1.f : 1.f;

		right = XMVectorSet(right.m128_f32[0] * velSign,
							0,
							right.m128_f32[2] * velSign,
							right.m128_f32[3] * velSign);
		right = XMVectorScale(XMVector3Normalize(right), moveVel);
	}

	const XMVECTOR posVec = DirectX::XMVectorAdd(XMLoadFloat3(&obj->getPos()),
												 right);

	XMFLOAT3 pos{};
	XMStoreFloat3(&pos, posVec);
	obj->setPos(pos);
}

void Player::shot(Camera *camera,
				  ObjModel *model,
				  const DirectX::XMFLOAT3 &vel) {

	// C++17から追加した要素の参照が返ってくるようになった
	PlayerBullet &i = bul.emplace_front(camera, model, XMFLOAT3(0, 0, 1));

	i.vel = vel;
	i.setPos(obj->getPos());
	i.setScale(XMFLOAT3(10, 10, 10));
}

void Player::drawWithUpdate(Light *light) {
	bul.remove_if([](PlayerBullet &i) {return !i.alive; });

	for (PlayerBullet &i : bul) {
		i.drawWithUpdate(light);
	}

	obj->drawWithUpdate(light);
}
