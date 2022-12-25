#include "BossEnemy.h"
#include "Util/RandomNum.h"

using namespace DirectX;

DirectX::XMVECTOR BossEnemy::calcVelVec(GameObj* me, bool moveYFlag)
{
	XMVECTOR velVec = XMLoadFloat3(&targetObj->getPos()) - XMLoadFloat3(&me->getPos());

	if (!moveYFlag)
	{
		// Y方向には移動しない
		velVec = XMVectorSetY(velVec, 0.f);
	}

	return velVec;
}

void BossEnemy::moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec)
{
	// 大きさを反映した速度をXMFLOAT3に変換
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, XMVector3Normalize(velVec) * moveSpeed);

	// 移動
	setVel(vel);

	// 速度に合わせて回転
	const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(vel);
	setRotation(XMFLOAT3(rotaDeg.x, rotaDeg.y, getRotation().z));
}

void BossEnemy::afterUpdate()
{
	// 死んだ弾は消す
	for (auto& i : smallEnemy)
	{
		i->setDrawFlag(i->getAlive());
	}
}

void BossEnemy::additionalDraw(Light* light)
{
	for (auto& i : smallEnemy)
	{
		i->drawWithUpdate(light);
	}
}

void BossEnemy::phase_approach()
{
	XMVECTOR velVec = calcVelVec(this);

	// 一定距離より近ければ遠ざかる
	if (XMVectorGetX(XMVector3Length(velVec)) < getScale())
	{
		// todo ここで近接攻撃を開始(攻撃関数へ遷移)
		addSmallEnemy();
		setPhase(std::bind(&BossEnemy::phase_leave, this));
		return;
	}

	// 大きさを反映
	moveAndRota(moveSpeed, velVec);
}

void BossEnemy::phase_leave()
{
	XMVECTOR velVec = calcVelVec(this);

	// 一定距離より遠ければ近づく
	if (XMVectorGetX(XMVector3Length(velVec)) > getScaleF3().x * 5.f)
	{
		// ここで遠距離攻撃を開始(攻撃関数へ遷移)
		setPhase(std::bind(&BossEnemy::phase_attack, this));
		nowShotFrame = shotInterval;
		shotNum = 0u;
		return;
	}

	// 大きさを反映
	moveAndRota(moveSpeed, -velVec);
}

void BossEnemy::phase_attack()
{
	if (nowShotFrame++ >= shotInterval)
	{
		for (uint32_t i = 0; i < shoEnemyNum; ++i)
		{
			addSmallEnemy();
		}
		nowShotFrame = 0u;

		if (shotNum++ >= shotNumMax)
		{
			shotNum = 0;
			setPhase(std::bind(&BossEnemy::phase_approach, this));
		}
	}
}

// 弾関係
void BossEnemy::addSmallEnemy()
{
	auto& i = smallEnemy.emplace_front();
	i.reset(new BaseEnemy(camera, smallEnemyModel));
	i->setScale(10.f);
	i->setParent(this->getParent());
	i->setPos(this->getPos());
	i->setHp(1u);

	const XMVECTOR right = XMVector3Rotate(XMVector3Normalize(calcVelVec(this, true)),
										   XMQuaternionRotationRollPitchYaw(RandomNum::getRandf(0.f, XM_PI),
																			XM_PIDIV2,
																			0.f));
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, right * smallEnemyMoveSpeed);
	i->setVel(vel);
	i->setPhase(
		[&]
		{
			XMVECTOR velVec = calcVelVec(i.get(), true);
		velVec = XMVector3Normalize(velVec);

		const XMVECTOR oldVec = XMVector3Normalize(XMLoadFloat3(&i->getVel()));

		velVec = smallEnemyMoveSpeed * XMVectorLerp(oldVec, velVec, 0.05f);

		XMFLOAT3 vel{};
		XMStoreFloat3(&vel, velVec);
		i->setVel(vel);
		}
		);
}