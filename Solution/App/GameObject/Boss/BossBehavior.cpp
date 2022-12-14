#include "BossBehavior.h"
#include <DirectXMath.h>

using namespace DirectX;

NODE_RESULT BossBehavior::phase_approach()
{
	// 違うフェーズなら実行しない
	if (phase != PHASE::APPROACH) { return NODE_RESULT::FAIL; }

	const XMVECTOR velVec = boss->calcVelVec(boss);

	// 一定距離より近ければ遠ざかる
	if (XMVectorGetX(XMVector3Length(velVec)) < boss->getScale())
	{
		// todo ここで近接攻撃を開始(攻撃関数へ遷移)
		boss->addSmallEnemy();
		phase = PHASE::LEAVE;
		return NODE_RESULT::SUCCESS;
	}

	// 大きさを反映
	boss->moveAndRota(boss->moveSpeed, velVec);

	return NODE_RESULT::SUCCESS;
}

NODE_RESULT BossBehavior::phase_leave()
{
	// 違うフェーズなら実行しない
	if (phase != PHASE::LEAVE) { return NODE_RESULT::FAIL; }

	const XMVECTOR velVec = boss->calcVelVec(boss);

	// 一定距離より遠ければ近づく
	if (XMVectorGetX(XMVector3Length(velVec)) > boss->getScaleF3().x * 5.f)
	{
		// ここで遠距離攻撃を開始(攻撃関数へ遷移)
		phase = PHASE::ATTACK;
		boss->nowShotFrame = boss->shotInterval;
		boss->shotNum = 0u;
		return NODE_RESULT::SUCCESS;;
	}

	// 大きさを反映
	boss->moveAndRota(boss->moveSpeed, -velVec);

	return NODE_RESULT::SUCCESS;
}

NODE_RESULT BossBehavior::phase_attack()
{
	// 違うフェーズなら実行しない
	if (phase != PHASE::ATTACK) { return NODE_RESULT::FAIL; }

	if (boss->nowShotFrame++ >= boss->shotInterval)
	{
		boss->addSmallEnemy();
		boss->nowShotFrame = 0u;

		if (boss->shotNum++ >= boss->shotNumMax)
		{
			boss->shotNum = 0;
			phase = PHASE::APPROACH;
		}
	}

	return NODE_RESULT::SUCCESS;
}

BossBehavior::BossBehavior(BossEnemy* boss) :
	boss(boss),
	rootNode(std::make_unique<Selector>()),
	phase(PHASE::APPROACH)
{
	// 各フェーズを登録
	rootNode->addChild(std::bind(&BossBehavior::phase_approach, this));
	rootNode->addChild(std::bind(&BossBehavior::phase_leave, this));
	rootNode->addChild(std::bind(&BossBehavior::phase_attack, this));
}

BossBehavior::BossBehavior() :
	BossBehavior(nullptr)
{
}
