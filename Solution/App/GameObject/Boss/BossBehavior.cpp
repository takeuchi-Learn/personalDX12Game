﻿#include "BossBehavior.h"
#include <GameObject/Boss/BossEnemy.h>
#include <DirectXMath.h>
#include <Util/Util.h>

using namespace DirectX;

NODE_RESULT BossBehavior::phase_Rotation()
{
	if (rotaPhaseNowCount++ > rotationPhaseData.countMax)
	{
		rotaPhaseNowCount = 0ui32;
		return NODE_RESULT::SUCCESS;
	}

	const float raito = (float)rotaPhaseNowCount / (float)rotationPhaseData.countMax;

	XMFLOAT3 rot{};
	rot.x = std::lerp(0.f, rotationPhaseData.rotaMax.x, raito);
	rot.y = std::lerp(0.f, rotationPhaseData.rotaMax.y, raito);
	rot.z = std::lerp(0.f, rotationPhaseData.rotaMax.z, raito);

	boss->setRotation(rot);

	return NODE_RESULT::RUNNING;
}

NODE_RESULT BossBehavior::phase_fanShapeAttack()
{
	// 撃つ時間がまだなら何もしない
	if (fanShotData.shotFrame.nowVal++ < fanShotData.shotFrame.MaxVal) { return NODE_RESULT::RUNNING; }
	fanShotData.shotFrame.nowVal = 0u;

	// 指定回数撃ったら次の行動へ
	if (fanShotData.count.nowVal >= fanShotData.count.MaxVal)
	{
		fanShotData.shotFrame.nowVal = fanShotData.shotFrame.MaxVal;
		fanShotData.count.nowVal = 0;
		return NODE_RESULT::SUCCESS;
	}
	++fanShotData.count.nowVal;

	// -これ~これの範囲で発射
	constexpr float angleMax = static_cast<float>(3.141592653589793 * (1.0 / 8.0));
	// 二回に一回ずらす値
	const float halfRad = angleMax / float(fanShotData.shotNum - 1u);

	// 攻撃対象へ向かうベクトル
	const XMVECTOR directionVec = boss->calcVelVec(boss, true);

	for (uint32_t i = 0ui32; i < fanShotData.shotNum; ++i)
	{
		// このfor文内での進行度
		const float raito = (float)i / float(fanShotData.shotNum - 1);

		// 射出角度
		XMFLOAT2 angle = XMFLOAT2(-angleMax, angleMax);
		// 二回に一回半分ずらす(これがないとあんま当たらん)
		if (fanShotData.count.nowVal & 1)
		{
			angle.x += halfRad;
			angle.y += halfRad;
		}

		// 弾の射出方向
		const XMVECTOR direction = XMVector3Rotate(directionVec,
												   XMQuaternionRotationRollPitchYaw(0.f,
																					std::lerp(angle.x,
																							  angle.y,
																							  raito),
																					0.f));

		// 指定方向に弾を発射
		boss->addBul(direction, XMFLOAT3(10, 100, 10), fanShotData.bulCol);
	}

	return NODE_RESULT::RUNNING;
}

BossBehavior::BossBehavior(BossEnemy* boss) :
	Sequencer(),
	boss(boss)
{
	const auto data = Util::loadCsv("Resources/fanShotData.csv");

	fanShotData =
		FanShotData{
		.shotFrame = {.MaxVal = std::stoul(data[0][0]), .nowVal = 0u },
		.count = {.MaxVal = std::stoul(data[1][0]), .nowVal = 0u},
		.shotNum = std::stoul(data[2][0]),
		.bulCol = XMFLOAT4(std::stof(data[3][0]),
						   std::stof(data[3][1]),
						   std::stof(data[3][2]),
						   std::stof(data[3][3]))
	};

	fanShotData.shotFrame.nowVal = fanShotData.shotFrame.MaxVal;

	// 各フェーズを登録
	addChild(Task(std::bind(&BossBehavior::phase_Rotation, this)));
	addChild(Task(std::bind(&BossBehavior::phase_fanShapeAttack, this)));
}

BossBehavior::BossBehavior() :
	BossBehavior(nullptr)
{}