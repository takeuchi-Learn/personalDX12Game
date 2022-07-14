#include "Player.h"

void Player::moveForward(float moveVel) {
	XMVECTOR moveVec =
		DirectX::XMVectorScale(DirectX::XMVector3Normalize(lookVec),
							   moveVel);

	pos = DirectX::XMVectorAdd(pos, moveVec);
}

void Player::moveRight(float moveVel, bool moveYFlag) {
	XMVECTOR moveVec =
		DirectX::XMVectorScale(DirectX::XMVector3Normalize(lookVec),
							   moveVel);

	XMFLOAT3 val{ moveVec.m128_f32[2], 0, -moveVec.m128_f32[0] };

	if (moveYFlag) {
		val.y = moveVec.m128_f32[1];
	}

	XMVECTOR valVec = DirectX::XMLoadFloat3(&val);

	pos = DirectX::XMVectorAdd(pos, valVec);
}
