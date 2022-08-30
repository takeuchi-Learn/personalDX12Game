#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

Player::Player(Camera *camera,
			   ObjModel *model,
			   const DirectX::XMFLOAT3 &pos)
	: GameObj(camera, model, pos),
	aimObjLen(20.f) {
	constexpr size_t aimObjNum = 2U;

	aimObj.resize(aimObjNum);

	for (size_t i = 0u; i < aimObjNum; ++i) {
		aimObj[i] = std::make_unique<Object3d>(camera, model, 0U);
		aimObj[i]->position = XMFLOAT3(0, 0, aimObjLen / float(i + 1u));
		aimObj[i]->parent = obj.get();
		const float scale = 1.f / float(aimObjNum - i + 1u);
		aimObj[i]->scale = XMFLOAT3(scale, scale, scale);
	}
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

	// 照準のある方向へ、速さvelで飛んでいく
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, XMVector3Transform(XMVectorSet(aimObj[0]->position.x / aimObjLen * speed,
													   aimObj[0]->position.y / aimObjLen * speed,
													   aimObj[0]->position.z / aimObjLen * speed,
													   1),
										   obj->getMatRota()));

	i.setVel(vel);
	i.setScale(bulScale);
}

void Player::update() {
	// 死んだ弾は消す
	bul.remove_if([](PlayerBullet &i) {return !i.getAlive(); });

	if (alive) {
		for (auto &i : aimObj) {
			i->update();
		}
	}
}

void Player::additionalDraw(Light *light) {
	if (alive) {
		for (auto &i : aimObj) {
			i->draw(DX12Base::getInstance(), light);
		}
	}

	for (auto &i : bul) {
		i.drawWithUpdate(light);
	}
}