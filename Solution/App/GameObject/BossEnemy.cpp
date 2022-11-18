#include "BossEnemy.h"

using namespace DirectX;

void BossEnemy::phase_approach(GameObj* targetObj)
{
	XMVECTOR velVec = XMLoadFloat3(&targetObj->getPos()) - XMLoadFloat3(&getPos());

	// Y方向には移動しない
	velVec = XMVectorSetY(velVec, 0.f);

	// 一定距離より近ければ移動しない
	if (XMVectorGetX(XMVector3Length(velVec)) < getScaleF3().x)
	{
		return;
	}

	// 大きさを反映
	constexpr float speed = 2.f;
	velVec = XMVector3Normalize(velVec) * speed;

	// XMFLOAT3に変換
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, velVec);

	// 移動
	move(vel);

	// 速度に合わせて回転
	const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(vel);
	setRotation(XMFLOAT3(rotaDeg.x, rotaDeg.y, 0.f));
}