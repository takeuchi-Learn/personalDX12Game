#include "BossBehavior.h"
#include <GameObject/Boss/BossEnemy.h>
#include <DirectXMath.h>
#include <Util/Util.h>
#include <Util/RandomNum.h>
#include <Util/Timer.h>
#include <Collision/Collision.h>

#include <ExternalCode/ini.h>

#include <fstream>

using namespace DirectX;

bool BossBehavior::loadShotDataFileIni()
{
	constexpr const char filePath[] = "Resources/bossShotData.ini";

	std::string data{};
	{
		std::ifstream ifs(filePath);
		if (!ifs) { return true; }

		std::string line{};
		while (std::getline(ifs, line))
		{
			data += line + "\n";
		}
		ifs.close();
	}

	Util::IniData iniData(data);

	loadFanShotDataIni(iniData.ini);
	loadSingleShotDataIni(iniData.ini);

	return false;
}

void BossBehavior::loadFanShotDataIni(ini_t* ini)
{
	const int section = ini_find_section(ini, "FanShotData", 0);

	int index = ini_find_property(ini, section, "shotFrameMaxVal", 0);
	std::string val = ini_property_value(ini, section, index);
	fanShotData->shotFrame.maxVal = std::stoul(val);
	fanShotData->shotFrame.nowVal = fanShotData->shotFrame.maxVal;

	index = ini_find_property(ini, section, "countMaxVal", 0);
	val = ini_property_value(ini, section, index);
	fanShotData->count = { .maxVal = std::stoul(val), .nowVal = 0u };

	index = ini_find_property(ini, section, "shotNum", 0);
	val = ini_property_value(ini, section, index);
	fanShotData->shotNum = std::stoul(val);

	index = ini_find_property(ini, section, "bulColR", 0);
	val = ini_property_value(ini, section, index);
	fanShotData->bulCol.x = std::stof(val);

	index = ini_find_property(ini, section, "bulColG", 0);
	val = ini_property_value(ini, section, index);
	fanShotData->bulCol.y = std::stof(val);

	index = ini_find_property(ini, section, "bulColB", 0);
	val = ini_property_value(ini, section, index);
	fanShotData->bulCol.z = std::stof(val);

	index = ini_find_property(ini, section, "bulColA", 0);
	val = ini_property_value(ini, section, index);
	fanShotData->bulCol.w = std::stof(val);
}

void BossBehavior::loadSingleShotDataIni(ini_t* ini)
{
	const int section = ini_find_section(ini, "SingleShotData", 0);

	int index = ini_find_property(ini, section, "shotFrameMaxVal", 0);
	std::string val = ini_property_value(ini, section, index);
	singleShotData->shotFrame.maxVal = std::stoul(val);
	singleShotData->shotFrame.nowVal = singleShotData->shotFrame.maxVal;

	index = ini_find_property(ini, section, "countMaxVal", 0);
	val = ini_property_value(ini, section, index);
	singleShotData->count = { .maxVal = std::stoul(val), .nowVal = 0u };

	index = ini_find_property(ini, section, "bulColR", 0);
	val = ini_property_value(ini, section, index);
	singleShotData->bulCol.x = std::stof(val);

	index = ini_find_property(ini, section, "bulColG", 0);
	val = ini_property_value(ini, section, index);
	singleShotData->bulCol.y = std::stof(val);

	index = ini_find_property(ini, section, "bulColB", 0);
	val = ini_property_value(ini, section, index);
	singleShotData->bulCol.z = std::stof(val);

	index = ini_find_property(ini, section, "bulColA", 0);
	val = ini_property_value(ini, section, index);
	singleShotData->bulCol.w = std::stof(val);
}

bool BossBehavior::loadShotDataFileYml()
{
	constexpr const char filePath[] = "Resources/DataFile/bossShotData.yml";

	std::string data{};
	{
		std::ifstream ifs(filePath);
		if (!ifs) { return true; }

		std::string line{};
		while (std::getline(ifs, line))
		{
			data += line + "\n";
		}
		ifs.close();
	}
	Yaml::Node root{};
	try
	{
		Yaml::Parse(root, data);
	} catch (...)
	{
		return true;
	}

	loadFanShotDataYml(root["FanShotData"]);
	loadSingleShotDataYml(root["SingleShotData"]);

	return false;
}

void BossBehavior::loadFanShotDataYml(Yaml::Node& root)
{
	fanShotData->count = { .maxVal = root["countMaxVal"].As<uint32_t>(), .nowVal = 0ui32 };
	fanShotData->shotFrame = { .maxVal = root["shotFrameMaxVal"].As<uint32_t>(), .nowVal = 0ui32 };
	fanShotData->shotNum = root["shotNum"].As<uint32_t>();
	fanShotData->bulCol = XMFLOAT4(root["bulCol"]["R"].As<float>(),
								   root["bulCol"]["G"].As<float>(),
								   root["bulCol"]["B"].As<float>(),
								   root["bulCol"]["A"].As<float>());
}

void BossBehavior::loadSingleShotDataYml(Yaml::Node& root)
{
	singleShotData->count = { .maxVal = root["countMaxVal"].As<uint32_t>(), .nowVal = 0ui32 };
	singleShotData->shotFrame = { .maxVal = root["shotFrameMaxVal"].As<uint32_t>(), .nowVal = 0ui32 };
	singleShotData->bulCol = XMFLOAT4(root["bulCol"]["R"].As<float>(),
									  root["bulCol"]["G"].As<float>(),
									  root["bulCol"]["B"].As<float>(),
									  root["bulCol"]["A"].As<float>());
}

NODE_RESULT BossBehavior::phase_Rotation(const DirectX::XMFLOAT3& rotaMin,
										 const DirectX::XMFLOAT3& rotaMax)
{
	if (rotationPhaseData.count.nowVal++ > rotationPhaseData.count.maxVal)
	{
		rotationPhaseData.count.nowVal = 0ui32;
		return NODE_RESULT::SUCCESS;
	}

	const float raito = (float)rotationPhaseData.count.nowVal / (float)rotationPhaseData.count.maxVal;

	const XMFLOAT3 rot(std::lerp(rotaMin.x, rotaMax.x, raito),
					   std::lerp(rotaMin.z, rotaMax.y, raito),
					   std::lerp(rotaMin.x, rotaMax.z, raito));
	boss->setRotation(rot);

	return NODE_RESULT::RUNNING;
}

NODE_RESULT BossBehavior::phase_fanShapeAttack()
{
	// 撃つ時間がまだなら何もしない
	if (fanShotData->shotFrame.nowVal++ < fanShotData->shotFrame.maxVal) { return NODE_RESULT::RUNNING; }
	fanShotData->shotFrame.nowVal = 0u;

	// 指定回数撃ったら次の行動へ
	if (fanShotData->count.nowVal >= fanShotData->count.maxVal)
	{
		fanShotData->shotFrame.nowVal = fanShotData->shotFrame.maxVal;
		fanShotData->count.nowVal = 0;
		return NODE_RESULT::SUCCESS;
	}
	++fanShotData->count.nowVal;

	// -これ~これの範囲で発射
	constexpr float angleMax = static_cast<float>(3.141592653589793 * (1.0 / 8.0));
	// 二回に一回ずらす値
	const float halfRad = angleMax / float(fanShotData->shotNum - 1u);

	// 攻撃対象へ向かうベクトル
	const XMVECTOR directionVec = boss->calcVelVec(boss, true);

	for (uint32_t i = 0ui32; i < fanShotData->shotNum; ++i)
	{
		// このfor文内での進行度
		const float raito = (float)i / float(fanShotData->shotNum - 1);

		// 射出角度
		XMFLOAT2 angle = XMFLOAT2(-angleMax, angleMax);
		// 二回に一回半分ずらす(これがないとあんま当たらん)
		if (fanShotData->count.nowVal & 1)
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
		constexpr XMFLOAT3 scale = XMFLOAT3(2.5f, 100, 2.5f);
		boss->addBul(direction, scale, fanShotData->bulCol, 2.f);
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

NODE_RESULT BossBehavior::phase_tornado()
{
	if (++tornadoPhaseData.frame.nowVal > tornadoPhaseData.frame.maxVal)
	{
		tornadoPhaseData.frame.nowVal = 0u;
		return NODE_RESULT::SUCCESS;
	}
	const float raito = (float)tornadoPhaseData.frame.nowVal / (float)tornadoPhaseData.frame.maxVal;

	XMFLOAT3 vel = GameObj::calcVel(tornadoPhaseData.tornadoWorldPos,
									boss->getTargetObj()->calcWorldPos(),
									tornadoPhaseData.targetSpeed);
	XMFLOAT3 pos = boss->getTargetObj()->getPos();
	pos.x += vel.x;
	pos.y += vel.y;
	pos.z += vel.z;
	boss->getTargetObj()->setPos(pos);

	constexpr auto particleLife = Timer::oneSec * 5;
	constexpr float randRange = XM_PIDIV4 / 2.f;
	constexpr float rotaRadMax = XM_2PI * 10.f;
	XMFLOAT3 particleVelAngleRad = XMFLOAT3(0, raito * rotaRadMax, 0);

	XMVECTOR particleVelVec = XMVectorSet(0, 5, 2, 0);
	particleVelVec = XMVector3Rotate(particleVelVec,
									 XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&particleVelAngleRad)));
	XMFLOAT3 particleVel{};
	XMStoreFloat3(&particleVel, particleVelVec);

	boss->tornadoParticle->add(particleLife,
							   tornadoPhaseData.tornadoWorldPos,
							   particleVel, XMFLOAT3(),
							   10.f, 0.f,
							   0.f, 0.f,
							   XMFLOAT3(1, 1, 1), XMFLOAT3(1, 1, 1));

	particleVelAngleRad.y += XM_PIDIV2;
	particleVelVec = XMVector3Rotate(XMVectorSet(0, 5, 2, 0),
									 XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&particleVelAngleRad)));
	XMStoreFloat3(&particleVel, particleVelVec);

	boss->tornadoParticle->add(particleLife,
							   tornadoPhaseData.tornadoWorldPos,
							   particleVel, XMFLOAT3(),
							   10.f, 0.f,
							   0.f, 0.f,
							   XMFLOAT3(1, 1, 1), XMFLOAT3(1, 1, 1));

	return NODE_RESULT::RUNNING;
}

NODE_RESULT BossBehavior::phase_setTornadoData()
{
	XMVECTOR backVec = XMVector3Rotate(XMVectorSet(0, 0, -1.f, 1.f),
									   XMQuaternionRotationRollPitchYaw(XMConvertToRadians(boss->getRotation().x),
																		XMConvertToRadians(boss->getRotation().y),
																		XMConvertToRadians(boss->getRotation().z)));

	backVec = XMVectorSetY(backVec, 0.f);
	backVec = XMVector3Normalize(backVec) * this->boss->getMaxTargetDistance() * 0.2f;

	XMFLOAT3 forward{};
	XMStoreFloat3(&forward, backVec);

	tornadoPhaseData.tornadoWorldPos = boss->calcWorldPos();
	tornadoPhaseData.tornadoWorldPos.x += forward.x;
	tornadoPhaseData.tornadoWorldPos.y = boss->getTargetObj()->calcWorldPos().y;
	tornadoPhaseData.tornadoWorldPos.z += forward.z;

	return NODE_RESULT::SUCCESS;
}

BossBehavior::BossBehavior(BossEnemy* boss) :
	Selector(),
	boss(boss),
	fanShotData(std::make_unique<FanShotData>()),
	singleShotData(std::make_unique<SingleShotData>())
{
	loadShotDataFileYml();

	// --------------------
	// 各フェーズを登録
	// --------------------

	// 扇形攻撃
	fanShapePhase = std::make_unique<Sequencer>();
	fanShapePhase->addChild(Task(std::bind(&BossBehavior::phase_fanShapeAttack, this)));

	// 連続単発攻撃
	singleShotPhase = std::make_unique<Sequencer>();
	singleShotPhase->addChild(Task(std::bind(&BossBehavior::phase_singleShotAttack, this)));

	// 引き寄せる
	tornadoPhase = std::make_unique<Sequencer>();
	tornadoPhase->addChild(Task(std::bind(&BossBehavior::phase_setTornadoData, this)));
	tornadoPhase->addChild(Task(std::bind(&BossBehavior::phase_tornado, this)));

	// 攻撃対象が近い時の行動
	nearTargetPhase = std::make_unique<Sequencer>();
	nearTargetPhase->addChild(Task([&] { return this->boss->calcTargetDistance() < this->boss->getMaxTargetDistance() * 0.5f ? NODE_RESULT::SUCCESS : NODE_RESULT::FAIL; }));
	nearTargetPhase->addChild(Task(std::bind(&BossBehavior::phase_Rotation, this, XMFLOAT3(0, 0, 0), XMFLOAT3(0, 720, 0))));
	nearTargetPhase->addChild(*fanShapePhase);

	// 攻撃対象が遠い時の行動
	farTargetPhase = std::make_unique<Sequencer>();
	farTargetPhase->addChild(Task([&] { return this->boss->calcTargetDistance() > this->boss->getMaxTargetDistance() * 0.5f ? NODE_RESULT::SUCCESS : NODE_RESULT::FAIL; }));
	farTargetPhase->addChild(Task(std::bind(&BossBehavior::phase_Rotation, this, XMFLOAT3(0, 0, 0), XMFLOAT3(360, -360, 0))));
	farTargetPhase->addChild(*tornadoPhase);
	farTargetPhase->addChild(*singleShotPhase);

	addChild(*nearTargetPhase);
	addChild(*farTargetPhase);
}

BossBehavior::BossBehavior() :
	BossBehavior(nullptr)
{}