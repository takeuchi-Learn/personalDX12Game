#pragma once
#include "BaseEnemy.h"
#include <forward_list>
class BossEnemy :
	public BaseEnemy
{
	// 攻撃対象へのポインタ
	GameObj* targetObj = nullptr;

	// 移動速度
	float moveSpeed = 2.f;

	ObjModel* smallEnemyModel = nullptr;
	std::forward_list<std::unique_ptr<BaseEnemy>> smallEnemy;

	DirectX::XMVECTOR calcVelVec(GameObj* me, bool moveYFlag = false);

	void moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec);

	void afterUpdate() override;
	void additionalDraw(Light* light) override;

public:
	using BaseEnemy::BaseEnemy;

	size_t calcSmallEnemyNum() const { return std::distance(smallEnemy.begin(), smallEnemy.end()); }

	inline const auto& getSmallEnemyList() const { return smallEnemy; }

	inline ObjModel* getSmallEnemyModel() { return smallEnemyModel; }
	inline void setSmallEnemyModel(ObjModel* model) { smallEnemyModel = model; }

	inline void setTargetObj(GameObj* obj) { targetObj = obj; }
	inline GameObj* getTargetObj() { return targetObj; }

	void addSmallEnemy();

	void phase_approach();
	inline void changePhase_approach() { setPhase(std::bind(&BossEnemy::phase_approach, this)); }

	void phase_leave();
};
