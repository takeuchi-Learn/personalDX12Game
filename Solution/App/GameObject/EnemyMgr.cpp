#include "EnemyMgr.h"

using namespace DirectX;

EnemyMgr::EnemyMgr(Camera* camera, Light* light) :
	camera(camera),
	light(light)
{
}

void EnemyMgr::addSmallEnemy(ObjModel* model, const DirectX::XMFLOAT3& pos)
{
	auto& i = smallEnemy.emplace_front(new BaseEnemy(camera, model, pos));
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
