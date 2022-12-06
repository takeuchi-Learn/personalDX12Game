#include "EnemyMgr.h"

using namespace DirectX;

void EnemyMgr::smallEnemy_Approach(BaseEnemy* enemy,
								   const DirectX::XMFLOAT3& vel)
{
	enemy->setVel(vel);
}

EnemyMgr::EnemyMgr(Camera* camera, Light* light) :
	camera(camera),
	light(light)
{
}

void EnemyMgr::addSmallEnemy(ObjModel* model, const DirectX::XMFLOAT3& pos)
{
	auto& i = smallEnemy.emplace_front(new BaseEnemy(camera, model, pos));

	constexpr XMFLOAT3 vel = XMFLOAT3(0, 0, -1);
	i->setPhase(std::bind(&EnemyMgr::smallEnemy_Approach, this,
						  i.get(), vel));
}

void EnemyMgr::update()
{
	for (auto& i : smallEnemy)
	{
		i->update();
	}
}

void EnemyMgr::draw()
{
	for (auto& i : smallEnemy)
	{
		i->draw(light);
	}
}

void EnemyMgr::drawWithUpdate()
{
	for (auto& i : smallEnemy)
	{
		i->drawWithUpdate(light);
	}
}