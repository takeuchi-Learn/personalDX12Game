﻿#include "BossEnemy.h"
#include "Util/RandomNum.h"
#include <Collision/Collision.h>

using namespace DirectX;

DirectX::XMVECTOR BossEnemy::calcVelVec(GameObj* me, bool moveYFlag)
{
	XMVECTOR velVec = XMLoadFloat3(&targetObj->calcWorldPos()) - XMLoadFloat3(&me->getPos());

	if (!moveYFlag)
	{
		// Y方向には移動しない
		velVec = XMVectorSetY(velVec, 0.f);
	}

	return velVec;
}

void BossEnemy::move(float moveSpeed, const DirectX::XMVECTOR& velVec, DirectX::XMFLOAT3* velBuf)
{
	// 大きさを反映した速度をXMFLOAT3に変換
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, XMVector3Normalize(velVec) * moveSpeed);

	// 移動
	setVel(vel);

	if (velBuf)
	{
		*velBuf = vel;
	}
}

void BossEnemy::moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec, DirectX::XMFLOAT3* velBuf)
{
	// 大きさを反映した速度
	XMFLOAT3 vel{};
	move(moveSpeed, velVec, &vel);

	if (velBuf)
	{
		*velBuf = vel;
	}

	// 速度に合わせて回転
	const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(vel);
	setRotation(XMFLOAT3(rotaDeg.x, rotaDeg.y, getRotation().z));
}

void BossEnemy::afterUpdate()
{
	// 死んだ弾は消す
	for (auto& i : bul)
	{
		i->setDrawFlag(i->getAlive());
	}
}

void BossEnemy::additionalDraw(Light* light)
{
	for (auto& i : bul)
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
	bossBehavior(std::make_unique<BossBehavior>(this)),
	tornadoParticle(std::make_unique<ParticleMgr>(L"Resources/white.png", camera))
{
	setPhase([&] { return bossBehavior->run(); });
}

float BossEnemy::calcTargetDistance()
{
	if (!this->targetObj) { return -1.f; }
	const XMVECTOR bpos = XMLoadFloat3(&this->calcWorldPos());
	const XMVECTOR tpos = XMLoadFloat3(&this->targetObj->calcWorldPos());

	return Collision::vecLength(bpos - tpos);
}

// 弾関係
void BossEnemy::addBulHoming(const DirectX::XMFLOAT4& color, float moveSpeed)
{
	auto& i = bul.emplace_front(std::make_shared<Bul>(camera, bulModel));
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
	XMStoreFloat3(&vel, right * moveSpeed);
	i->setVel(vel);
	i->setPhase(
		[&]
		{
			XMVECTOR velVec = calcVelVec(i.get(), true);
			velVec = XMVector3Normalize(velVec);

			const XMVECTOR oldVec = XMVector3Normalize(XMLoadFloat3(&i->getVel()));

			velVec = moveSpeed * XMVectorLerp(oldVec, velVec, 0.05f);

			XMFLOAT3 vel{};
			XMStoreFloat3(&vel, velVec);
			i->setVel(vel);
		}
		);
}

void BossEnemy::addBul(const DirectX::XMVECTOR& direction,
					   const DirectX::XMFLOAT3& scale,
					   const DirectX::XMFLOAT4& color,
					   float moveSpeed)
{
	// 0ベクトルだと向きが無いので除外
	assert(!XMVector3Equal(direction, XMVectorZero()));
	assert(!XMVector3IsInfinite(direction));

	auto& i = bul.emplace_front(std::make_shared<Bul>(camera, bulModel));
	i->setScaleF3(scale);
	i->setParent(this->getParent());
	i->setPos(this->getPos());
	i->setHp(1u);
	i->setLife(bulLife);
	i->setCol(color);

	XMVECTOR velVec = moveSpeed * XMVector3Normalize(direction);
	XMFLOAT3 vel{};
	XMStoreFloat3(&vel, velVec);
	i->setVel(vel);
}

void BossEnemy::Bul::afterUpdate()
{
	if (alive)
	{
		life == 0u ? kill() : --life;
	}
}