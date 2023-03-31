#include "BossBehavior.h"
#include <GameObject/Boss/BossEnemy.h>
#include <DirectXMath.h>
#include <Util/Util.h>
#include <Collision/Collision.h>

using namespace DirectX;

NODE_RESULT BossBehavior::phase_Rotation(const DirectX::XMFLOAT3& rotaMin,
										 const DirectX::XMFLOAT3& rotaMax)
{
	if (rotationPhaseData.count.nowVal++ > rotationPhaseData.count.maxVal)
	{
		rotationPhaseData.count.nowVal = 0ui32;
		return NODE_RESULT::SUCCESS;
	}

	const float raito = (float)rotationPhaseData.count.nowVal / (float)rotationPhaseData.count.maxVal;

	XMFLOAT3 rot{};
	rot.x = std::lerp(rotaMin.x, rotaMax.x, raito);
	rot.y = std::lerp(rotaMin.z, rotaMax.y, raito);
	rot.z = std::lerp(rotaMin.x, rotaMax.z, raito);

	boss->setRotation(rot);

	return NODE_RESULT::RUNNING;
}

NODE_RESULT BossBehavior::phase_fanShapeAttack()
{
	// 撃つ時間がまだなら何もしない
	if (fanShotData.shotFrame.nowVal++ < fanShotData.shotFrame.maxVal) { return NODE_RESULT::RUNNING; }
	fanShotData.shotFrame.nowVal = 0u;

	// 指定回数撃ったら次の行動へ
	if (fanShotData.count.nowVal >= fanShotData.count.maxVal)
	{
		fanShotData.shotFrame.nowVal = fanShotData.shotFrame.maxVal;
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
		constexpr XMFLOAT3 scale = XMFLOAT3(5, 100, 5);
		boss->addBul(direction, scale, fanShotData.bulCol, 2.f);
	}

	return NODE_RESULT::RUNNING;
}

NODE_RESULT BossBehavior::phase_singleShotAttack()
{
	if (singleShotData->shotFrame.nowVal++ < singleShotData->shotFrame.maxVal) { return NODE_RESULT::RUNNING; }

	if (singleShotData->count.nowVal >= singleShotData->count.maxVal)
	{
		singleShotData->shotFrame.nowVal = singleShotData->shotFrame.maxVal;
		singleShotData->count.nowVal = 0;
		return NODE_RESULT::SUCCESS;
	}
	singleShotData->shotFrame.nowVal = 0u;
	++singleShotData->count.nowVal;

	// 攻撃対象へ向かうベクトル
	const XMVECTOR directionVec = boss->calcVelVec(boss, true);

	constexpr float scaleVal = 10.f;
	constexpr XMFLOAT3 scale = XMFLOAT3(scaleVal, scaleVal, scaleVal);
	constexpr float speed = 10.f;
	boss->addBul(directionVec, scale, singleShotData->bulCol, speed);

	return NODE_RESULT::RUNNING;
}

BossBehavior::BossBehavior(BossEnemy* boss) :
	Selector(),
	boss(boss)
{
	const auto data = Util::loadCsv("Resources/fanShotData.csv");

	fanShotData =
		FanShotData{
		.shotFrame = {.maxVal = std::stoul(data[0][0]), .nowVal = 0u },
		.count = {.maxVal = std::stoul(data[1][0]), .nowVal = 0u},
		.shotNum = std::stoul(data[2][0]),
		.bulCol = XMFLOAT4(std::stof(data[3][0]),
						   std::stof(data[3][1]),
						   std::stof(data[3][2]),
						   std::stof(data[3][3]))
	};

	fanShotData.shotFrame.nowVal = fanShotData.shotFrame.maxVal;

	singleShotData = std::make_unique<SingleShotData>(
		SingleShotData{
			.shotFrame = {.maxVal = 60, .nowVal = 0},
			.count = {.maxVal = 5, .nowVal = 0},
			.bulCol = XMFLOAT4(std::stof(data[3][0]),
							   std::stof(data[3][1]),
							   std::stof(data[3][2]),
							   std::stof(data[3][3]))
		}
	);

	// --------------------
	// 各フェーズを登録
	// --------------------

	// 扇形攻撃
	fanShapePhase = std::make_unique<Sequencer>();
	fanShapePhase->addChild(Task(std::bind(&BossBehavior::phase_Rotation, this, XMFLOAT3(0, 0, 0), XMFLOAT3(0, 720, 0))));
	fanShapePhase->addChild(Task(std::bind(&BossBehavior::phase_fanShapeAttack, this)));

	// 連続単発攻撃
	singleShotPhase = std::make_unique<Sequencer>();
	singleShotPhase->addChild(Task(std::bind(&BossBehavior::phase_Rotation, this, XMFLOAT3(0, 0, 0), XMFLOAT3(360, -360, 0))));
	singleShotPhase->addChild(Task(std::bind(&BossBehavior::phase_singleShotAttack, this)));

	// 攻撃対象が近い時の行動
	nearTargetPhase = std::make_unique<Sequencer>();
	nearTargetPhase->addChild(Task([&] { return this->boss->calcTargetDistance() < this->boss->getMaxTargetDistance() * 0.625f ? NODE_RESULT::SUCCESS : NODE_RESULT::FAIL; }));
	nearTargetPhase->addChild(*fanShapePhase);

	// 攻撃対象が遠い時の行動
	farTargetPhase = std::make_unique<Sequencer>();
	farTargetPhase->addChild(Task([&] { return this->boss->calcTargetDistance() > this->boss->getMaxTargetDistance() * 0.625f ? NODE_RESULT::SUCCESS : NODE_RESULT::FAIL; }));
	farTargetPhase->addChild(*singleShotPhase);

	addChild(*nearTargetPhase);
	addChild(*farTargetPhase);
}

BossBehavior::BossBehavior() :
	BossBehavior(nullptr)
{}