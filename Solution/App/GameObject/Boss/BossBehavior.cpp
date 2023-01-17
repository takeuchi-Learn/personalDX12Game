﻿#include "BossBehavior.h"
#include <GameObject/Boss/BossEnemy.h>
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
		boss->addSmallEnemyHoming(bulCol);
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
		nowShotFrame = shotInterval;
		shotCount = 0u;
		return NODE_RESULT::SUCCESS;
	}

	// 大きさを反映
	boss->moveAndRota(boss->moveSpeed, -velVec);

	return NODE_RESULT::SUCCESS;
}

NODE_RESULT BossBehavior::phase_attack()
{
	// 違うフェーズなら実行しない
	if (phase != PHASE::ATTACK) { return NODE_RESULT::FAIL; }

	if (nowShotFrame++ >= shotInterval)
	{
		const XMVECTOR directionVec = boss->calcVelVec(boss, true);

		constexpr float angleMax = static_cast<float>(3.141592653589793 * (1.0 / 8.0));

		constexpr float oneRad = angleMax * 2.f / float(shotEnemyNum - 1);
		constexpr float halfRad = oneRad / 2.f;

		for (uint32_t i = 0ui32; i < shotEnemyNum; ++i)
		{
			// このfor文内での進行度
			const float raito = (float)i / float(shotEnemyNum - 1);

			// 射出角度
			// 二回に一回半分ずらす(これがないとあんま当たらん)
			const float angle = angleMax + halfRad * float(1ui32 - (shotCount & 1));

			// 弾の射出方向
			const XMVECTOR direction = XMVector3Rotate(directionVec,
													   XMQuaternionRotationRollPitchYaw(0.f,
																						std::lerp(-angle, angle, raito),
																						0.f));

			boss->addSmallEnemy(direction, bulCol);
		}
		nowShotFrame = 0u;

		if (shotCount < shotCountMax)
		{
			++shotCount;
		} else
		{
			shotCount = 0;
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