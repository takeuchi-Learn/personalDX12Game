#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

void Player::moveForward(float moveVel, bool moveYFlag) {
	XMVECTOR moveVec =
		DirectX::XMVectorScale(DirectX::XMVector3Normalize(lookVec),
							   moveVel);

	if (!moveYFlag) {
		moveVec = XMVectorSetY(moveVec, 0.f);
	}

	pos = DirectX::XMVectorAdd(pos, moveVec);
}

void Player::moveRight(float moveVel, bool moveYFlag) {
	const XMVECTOR look = DirectX::XMVector3Normalize(lookVec);

	XMVECTOR moveVec =
		DirectX::XMVectorScale(XMVector3Rotate(look, XMQuaternionRotationRollPitchYaw(0, XM_PIDIV2, 0)),
							   moveVel);

	if (!moveYFlag) {
		moveVec = XMVectorSetY(moveVec, 0.f);
	}

	pos = DirectX::XMVectorAdd(pos, moveVec);
}
