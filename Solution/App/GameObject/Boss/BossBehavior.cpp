#include "BossBehavior.h"
#include <GameObject/Boss/BossEnemy.h>
#include <DirectXMath.h>

using namespace DirectX;

NODE_RESULT BossBehavior::phase_approach()
{
	const XMVECTOR velVec = boss->calcVelVec(boss);

	// 一定距離近づけば次の行動へ
	if (XMVectorGetX(XMVector3Length(velVec)) < boss->getScale())
	{
		// 弾を一つ撃って次の行動へ進む
		boss->addSmallEnemyHoming(bulCol);
		return NODE_RESULT::SUCCESS;
	}

	// 接近する
	boss->moveAndRota(boss->moveSpeed, velVec);

	return NODE_RESULT::RUNNING;
}

NODE_RESULT BossBehavior::phase_leave()
{
	const XMVECTOR velVec = boss->calcVelVec(boss);

	// 一定距離離れたら次の行動へ
	if (XMVectorGetX(XMVector3Length(velVec)) > boss->getScaleF3().x * 5.f)
	{
		return NODE_RESULT::SUCCESS;
	}

	// 離れていく
	boss->move(boss->moveSpeed, -velVec);

	// 移動方向に合わせて回転
	XMFLOAT3 velF3{};
	XMStoreFloat3(&velF3, velVec);
	const XMFLOAT2 rotaDeg = GameObj::calcRotationSyncVelDeg(velF3);
	boss->setRotation(XMFLOAT3(rotaDeg.x, rotaDeg.y, boss->getRotation().z));

	return NODE_RESULT::RUNNING;
}

NODE_RESULT BossBehavior::phase_attack()
{
	// 撃つ時間がまだなら何もしない
	if (nowShotFrame++ < shotInterval) { return NODE_RESULT::RUNNING; }
	nowShotFrame = 0u;

	// 指定回数撃ったら次の行動へ
	if (shotCount >= shotCountMax)
	{
		nowShotFrame = shotInterval;
		shotCount = 0;
		return NODE_RESULT::SUCCESS;
	}
	++shotCount;

	// -これ~これの範囲で発射
	constexpr float angleMax = static_cast<float>(3.141592653589793 * (1.0 / 8.0));
	// 二回に一回ずらす値
	constexpr float halfRad = angleMax / float(shotEnemyNum - 1);

	// 攻撃対象へ向かうベクトル
	const XMVECTOR directionVec = boss->calcVelVec(boss, true);

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

	return NODE_RESULT::RUNNING;
}

BossBehavior::BossBehavior(BossEnemy* boss) :
	Sequencer(),
	boss(boss),
	nowShotFrame(shotInterval),
	shotCount(0u)
{
	// 各フェーズを登録
	addChild(Task(std::bind(&BossBehavior::phase_approach, this)));
	addChild(Task(std::bind(&BossBehavior::phase_leave, this)));
	addChild(Task(std::bind(&BossBehavior::phase_attack, this)));
}

BossBehavior::BossBehavior() :
	BossBehavior(nullptr)
{}