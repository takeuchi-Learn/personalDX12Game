#include "NormalEnemy.h"

using namespace DirectX;

NormalEnemy::NormalEnemy(Camera* camera,
						 ObjModel* model,
						 ObjModel* bulModel,
						 const DirectX::XMFLOAT3& pos)
	: BaseEnemy(camera, model, pos),
	bulModel(bulModel),
	camera(camera)
{
	obj->rotation.y += 180.f;
	phase = std::bind(&NormalEnemy::phase_Approach, this);
}

void NormalEnemy::shot(const DirectX::XMFLOAT3& targetPos,
					   float vel,
					   float bulScale)
{
	// C++17から追加した要素の参照が返ってくるようになった
	std::unique_ptr<EnemyBullet>& i = bul.emplace_front(new EnemyBullet(camera,
																		bulModel,
																		obj->position));
	// 親を設定
	i->setParent(obj->parent);

	// 大きさを設定
	i->setScaleF3(XMFLOAT3(bulScale * 2.f,
						   bulScale * 2.f,
						   bulScale));

	// 色を設定
	i->setCol(XMFLOAT4(0.25f, 1.f, 0.5f, 1.f));

	// 速度を算出
	const XMFLOAT3 velF3 = calcVel(targetPos, obj->position, vel);
	i->setVel(velF3);

	// 速度の向きに合わせて回転
	const XMFLOAT2 rota = calcRotationSyncVelDeg(velF3);
	i->setRotation(XMFLOAT3(rota.x,
							rota.y,
							obj->rotation.z));
}

#pragma region phase

void NormalEnemy::phase_Approach()
{
	obj->position.x += vel.x;
	obj->position.y += vel.y;
	obj->position.z += vel.z;

	if (targetObjPt && shotFrame-- == 0U)
	{
		shot(targetObjPt->getPos(), 1.f, 2.5f);
		shotFrame = shotFrameMax;
	}

	if (getPos().z < 0.f)
	{
		chansePhase_Leave(XMFLOAT3(1.f, 1.f, 0.f));
	}
}

void NormalEnemy::phase_Leave()
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

void NormalEnemy::afterUpdate()
{
	++nowFrame;

	// 生きている時のみ表示
	drawFlag = alive;

	bul.remove_if([](std::unique_ptr<EnemyBullet>& i) { return !i->getAlive(); });
}

void NormalEnemy::additionalDraw(Light* light)
{
	for (auto& i : bul)
	{
		i->drawWithUpdate(light);
	}
}