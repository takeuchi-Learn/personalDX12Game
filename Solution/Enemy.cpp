#include "Enemy.h"

using namespace DirectX;

Enemy::Enemy(Camera* camera,
			 ObjModel* model,
			 ObjModel* bulModel,
			 const DirectX::XMFLOAT3& pos)
	: GameObj(camera, model, pos),
	bulModel(bulModel),
	camera(camera),
	phase(std::bind(&Enemy::phase_Approach, this))
{
	obj->rotation.x += 180.f;
}

void Enemy::shot(const DirectX::XMFLOAT3& targetPos,
				 float vel,
				 float bulScale)
{
	// C++17から追加した要素の参照が返ってくるようになった
	std::unique_ptr<EnemyBullet>& i = bul.emplace_front(new EnemyBullet(camera,
																		bulModel,
																		obj->position));
	// 親を設定
	i->setParent(obj->parent);

	// わかりやすいように細長くする
	constexpr float bulScaleZ = 3.f;
	i->setScaleF3(XMFLOAT3(bulScale,
						   bulScale,
						   bulScale * bulScaleZ));

	// 速度を算出
	const XMFLOAT3 velF3 = calcVel(targetPos, obj->position, vel);
	i->setVel(velF3);

	// 速度の向きに合わせて回転
	{
		const XMFLOAT2 rota = calcRotationSyncVelDeg(velF3);
		i->setRotation(XMFLOAT3(rota.x,
								rota.y,
								obj->rotation.z));
	}
}

#pragma region phase

void Enemy::phase_Approach()
{
	obj->position.x += vel.x;
	obj->position.y += vel.y;
	obj->position.z += vel.z;

	if (targetObjPt != nullptr && shotFrame-- == 0U)
	{
		shot(targetObjPt->getPos(), 1.f, 2.5f);
		shotFrame = shotFrameMax;
	}
}

void Enemy::phase_Leave()
{
	obj->position.x += vel.x;
	obj->position.y += vel.y;
	obj->position.z += vel.z;

	if (std::abs(obj->position.x) >= 50.f &&
		std::abs(obj->position.y) >= 50.f)
	{
		kill();
	}
}

#pragma endregion phase

void Enemy::additionalUpdate()
{
	if (alive)
	{
		phase();
	}

	++nowFrame;

	bul.remove_if([](std::unique_ptr<EnemyBullet>& i) { return !i->getAlive(); });

	if (targetObjPt != nullptr)
	{
		// 補間する割合[0~1]
		// 1だと回避不可能
		// 調整項目
		constexpr float raito = 0.02f;
		for (auto& i : bul)
		{
			const XMFLOAT3 nowVel = i->getVel();

			// 速度の差分を取得
			XMFLOAT3 nextVel = calcVel(targetObjPt->getPos(), i->getPos(), 2.f);
			const float velLen = sqrtf(nextVel.x * nextVel.x +
									   nextVel.y * nextVel.y +
									   nextVel.z * nextVel.z);

			nextVel.x /= velLen;
			nextVel.y /= velLen;
			nextVel.z /= velLen;

			nextVel.x -= nowVel.x;
			nextVel.y -= nowVel.y;
			nextVel.z -= nowVel.z;

			// 速度の補間の割合を適用
			nextVel.x *= velLen * raito;
			nextVel.y *= velLen * raito;
			nextVel.z *= velLen * raito;

			// 前の速度に速度の差分を加算
			nextVel.x += nowVel.x;
			nextVel.y += nowVel.y;
			nextVel.z += nowVel.z;

			// 求めた速度を適用
			i->setVel(nextVel);

			// 進む向きに合わせて回転
			const XMFLOAT2 rota = calcRotationSyncVelDeg(nextVel);
			i->setRotation(XMFLOAT3(rota.x,
									rota.y,
									obj->rotation.z));
		}
	}
}

void Enemy::additionalDraw(Light* light)
{
	for (auto& i : bul)
	{
		i->drawWithUpdate(light);
	}
}