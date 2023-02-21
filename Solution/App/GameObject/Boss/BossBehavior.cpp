#include "BossBehavior.h"
#include <GameObject/Boss/BossEnemy.h>
#include <DirectXMath.h>

using namespace DirectX;

NODE_RESULT BossBehavior::phase_approach()
{
	// 違うフェーズなら実行しない
	if (phase != &BossBehavior::phase_approach) { return NODE_RESULT::FAIL; }

	const XMVECTOR velVec = boss->calcVelVec(boss);

	// 一定距離より近ければ遠ざかる
	if (XMVectorGetX(XMVector3Length(velVec)) < boss->getScale())
	{
		// todo ここで近接攻撃を開始(攻撃関数へ遷移)
		boss->addSmallEnemyHoming(bulCol);
		phase = &BossBehavior::phase_leave;
		return NODE_RESULT::SUCCESS;
	}

	// 大きさを反映
	boss->moveAndRota(boss->moveSpeed, velVec);

	return NODE_RESULT::SUCCESS;
}

NODE_RESULT BossBehavior::phase_leave()
{
	// 違うフェーズなら実行しない
	if (phase != &BossBehavior::phase_leave) { return NODE_RESULT::FAIL; }

	const XMVECTOR velVec = boss->calcVelVec(boss);

	// 一定距離より遠ければ近づく
	if (XMVectorGetX(XMVector3Length(velVec)) > boss->getScaleF3().x * 5.f)
	{
		// ここで遠距離攻撃を開始(攻撃関数へ遷移)
		phase = &BossBehavior::phase_attack;
		nowShotFrame = shotInterval;
		shotCount = 0u;
		return NODE_RESULT::SUCCESS;
	}

	// 大きさを反映
	boss->move(boss->moveSpeed, -velVec);

	// 速度に合わせて回転
	XMFLOAT3 velF3{};
	XMStoreFloat3(&velF3, velVec);
	const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(velF3);
	boss->setRotation(XMFLOAT3(rotaDeg.x, rotaDeg.y, boss->getRotation().z));

	return NODE_RESULT::SUCCESS;
}

NODE_RESULT BossBehavior::phase_attack()
{
	// 違うフェーズなら実行しない
	if (phase != &BossBehavior::phase_attack) { return NODE_RESULT::FAIL; }

	if (nowShotFrame++ >= shotInterval)
	{
		const XMVECTOR directionVec = boss->calcVelVec(boss, true);

		// -これ~これの範囲で発射
		constexpr float angleMax = static_cast<float>(3.141592653589793 * (1.0 / 8.0));
		constexpr float angleMaxDeg = XMConvertToDegrees(angleMax);
		// 発射する範囲の角度
		constexpr float allAngleDeg = angleMaxDeg * 2.f;

		constexpr float oneRad = angleMax * 2.f / float(shotEnemyNum - 1);
		constexpr float halfRad = oneRad / 2.f;

		for (uint32_t i = 0ui32; i < shotEnemyNum; ++i)
		{
			// このfor文内での進行度
			const float raito = (float)i / float(shotEnemyNum - 1);

			// 射出角度
			XMFLOAT2 angle = XMFLOAT2(-angleMax, angleMax);
			// 二回に一回半分ずらす(これがないとあんま当たらん)
			if (shotCount & 1)
			{
				angle.x += halfRad;
				angle.y += halfRad;
			}

			// 弾の射出方向
			const XMVECTOR direction = XMVector3Rotate(directionVec,
													   XMQuaternionRotationRollPitchYaw(0.f,
																						std::lerp(angle.x, angle.y, raito),
																						0.f));

			// 指定方向に弾を発射
			boss->addSmallEnemy(direction, bulCol);
		}
		// 全部打ち終わったらフレームをリセット
		nowShotFrame = 0u;

		// 指定回数打ったか判定
		if (shotCount < shotCountMax)
		{
			++shotCount;
		} else
		{
			// 指定回数打っていたらフェーズを変える
			shotCount = 0;
			phase = &BossBehavior::phase_approach;
		}
	}

	return NODE_RESULT::SUCCESS;
}

BossBehavior::BossBehavior(BossEnemy* boss) :
	Selector(),
	boss(boss),
	phase(&BossBehavior::phase_approach)
{
	// 各フェーズを登録
	addChild(Task(std::bind(phase, this)));
	addChild(Task(std::bind(&BossBehavior::phase_leave, this)));
	addChild(Task(std::bind(&BossBehavior::phase_attack, this)));
}

BossBehavior::BossBehavior() :
	BossBehavior(nullptr)
{}