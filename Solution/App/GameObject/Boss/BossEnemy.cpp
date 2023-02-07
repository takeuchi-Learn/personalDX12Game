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

void BossEnemy::move(float moveSpeed, const DirectX::XMVECTOR& velVec)
{
	// 大きさを反映した速度をXMFLOAT3に変換
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, XMVector3Normalize(velVec) * moveSpeed);

	// 移動
	setVel(vel);
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

BossEnemy::BossEnemy(Camera* camera,
					 ObjModel* model,
					 const DirectX::XMFLOAT3& pos,
					 uint16_t hp) : BaseEnemy(camera,
											  model,
											  pos,
											  hp),
	bossBehavior(std::make_unique<BossBehavior>(this))
{
	setPhase([&] { return bossBehavior->run(); });
}

// 弾関係
void BossEnemy::addSmallEnemyHoming(const DirectX::XMFLOAT4& color)
{
	auto& i = smallEnemy.emplace_front(std::make_shared<BaseEnemy>(camera, smallEnemyModel));
	i->setScale(10.f);
	i->setParent(this->getParent());
	i->setPos(this->getPos());
	i->setHp(1u);
	i->setLife(bulLife);
	i->setCol(color);

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

void BossEnemy::addSmallEnemy(const DirectX::XMVECTOR& direction,
							  const DirectX::XMFLOAT4& color)
{
	// 0ベクトルだと向きが無いので除外
	assert(!XMVector3Equal(direction, XMVectorZero()));
	assert(!XMVector3IsInfinite(direction));

	auto& i = smallEnemy.emplace_front(std::make_shared<BaseEnemy>(camera, smallEnemyModel));
	i->setScale(10.f);
	i->setParent(this->getParent());
	i->setPos(this->getPos());
	i->setHp(1u);
	i->setLife(bulLife);
	i->setCol(color);

	XMVECTOR velVec = smallEnemyMoveSpeed * XMVector3Normalize(direction);
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, velVec);
	i->setVel(vel);
}