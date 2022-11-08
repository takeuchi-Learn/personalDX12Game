#pragma once
#include "BaseEnemy.h"
#include <forward_list>
#include <memory>
#include <DirectXMath.h>
class EnemyMgr
{
private:
	Camera* camera = nullptr;
	Light* light = nullptr;

	std::forward_list <std::unique_ptr<BaseEnemy>> smallEnemy;

private:
	void smallEnemy_Approach(BaseEnemy* enemy,
							 const DirectX::XMFLOAT3& vel);

public:
	EnemyMgr(Camera* camera, Light* light);

	void addSmallEnemy(ObjModel* model, const DirectX::XMFLOAT3& pos = { 0,0,0 });

	void update();

	void draw();

	void drawWithUpdate();
};

