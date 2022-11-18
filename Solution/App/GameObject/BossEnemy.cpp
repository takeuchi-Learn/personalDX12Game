#include "BossEnemy.h"

using namespace DirectX;

DirectX::XMVECTOR BossEnemy::calcVelVec(GameObj* targetObj)
{
	XMVECTOR velVec = XMLoadFloat3(&targetObj->getPos()) - XMLoadFloat3(&getPos());

	// Y方向には移動しない
	return XMVectorSetY(velVec, 0.f);
}

void BossEnemy::moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec)
{
	// 大きさを反映した速度をXMFLOAT3に変換
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, XMVector3Normalize(velVec) * moveSpeed);

	// 移動
	move(vel);

	// 速度に合わせて回転
	const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(vel);
	setRotation(XMFLOAT3(rotaDeg.x, rotaDeg.y, 0.f));
}

void BossEnemy::phase_approach(GameObj* targetObj)
{
	XMVECTOR velVec = calcVelVec(targetObj);

	// 一定距離より近ければ遠ざかる
	if (XMVectorGetX(XMVector3Length(velVec)) < getScaleF3().x)
	{
		setPhase(std::bind(&BossEnemy::phase_leave, this, targetObj));
		return;
	}

	// 大きさを反映
	moveAndRota(moveSpeed, velVec);
}

void BossEnemy::phase_leave(GameObj* targetObj)
{
	XMVECTOR velVec = calcVelVec(targetObj);

	// 一定距離より遠ければ近づく
	if (XMVectorGetX(XMVector3Length(velVec)) > getScaleF3().x * 5.f)
	{
		setPhase(std::bind(&BossEnemy::phase_approach, this, targetObj));
		return;
	}

	// 大きさを反映
	moveAndRota(moveSpeed, -velVec);
}
