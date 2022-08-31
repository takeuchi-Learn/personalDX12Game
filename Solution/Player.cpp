#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

Player::Player(Camera *camera,
			   ObjModel *model,
			   const DirectX::XMFLOAT3 &pos)
	: GameObj(camera, model, pos),
	aimObjLen(20.f),
	showAimObjFlag(true) {
	constexpr size_t aimObjNum = 1;

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

void Player::additionalUpdate() {
	// 死んだ弾は消す
	bul.remove_if([](PlayerBullet &i) {return !i.getAlive(); });

	if (alive && showAimObjFlag) {
		{
			const XMMATRIX invMatVPV = XMMatrixInverse(nullptr, obj->getCamera()->getMatVPV());

			XMVECTOR nearPos = XMVector3Transform(XMVectorSet(aim2DPos.x,
															  aim2DPos.y,
															  0.f,
															  1.f),
												  invMatVPV);
			nearPos /= XMVectorGetW(nearPos);

			XMVECTOR farPos = XMVector3Transform(XMVectorSet(aim2DPos.x,
															 aim2DPos.y,
															 1.f,
															 1.f),
												 invMatVPV);
			farPos /= XMVectorGetW(farPos);

			const XMVECTOR near2Far = farPos - nearPos;

			const XMVECTOR aimObjPos = XMVector3Rotate(
				XMVector3Normalize(near2Far),
				XMQuaternionRotationRollPitchYaw(XMConvertToRadians(-obj->rotation.x),
												 XMConvertToRadians(-obj->rotation.y),
												 XMConvertToRadians(-obj->rotation.z)));
			XMFLOAT3 aimObjPosF3{};

			for (UINT i = 0u, len = (UINT)aimObj.size(); i < len; ++i) {
				XMStoreFloat3(&aimObjPosF3, aimObjPos * aimObjLen);
				aimObj[i]->position = aimObjPosF3;
			}

		}
		for (auto &i : aimObj) {
			i->update();
		}
	}
}

void Player::additionalDraw(Light *light) {
	if (alive && showAimObjFlag) {
		for (auto &i : aimObj) {
			i->draw(DX12Base::getInstance(), light);
		}
	}

	for (auto &i : bul) {
		i.drawWithUpdate(light);
	}
}