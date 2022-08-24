#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

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
				  float vel,
				  float bulScale) {

	// C++17から追加した要素の参照が返ってくるようになった
	PlayerBullet &i = bul.emplace_front(camera, model, obj->position);

	// Z方向のベクトルを、自機の向いている向きに回転
	XMFLOAT3 velF3{};
	XMStoreFloat3(&velF3, XMVector3Transform(XMVectorSet(0, 0, vel, 1), obj->getMatRota()));

	i.setVel(velF3);
	i.setScale(bulScale);
}

void Player::update() {
	// 死んだ弾は消す
	bul.remove_if([](PlayerBullet &i) {return !i.getAlive(); });
}

void Player::additionalDraw(Light *light) {
	for (auto &i : bul) {
		i.drawWithUpdate(light);
	}
}