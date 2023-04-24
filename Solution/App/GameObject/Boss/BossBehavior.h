/*****************************************************************//**
 * \file   BossBehavior.h
 * \brief  ボスの行動
 *********************************************************************/

#pragma once

#include <BehaviorTree/Selector.h>
#include <BehaviorTree/Sequencer.h>
#include <memory>
#include <DirectXMath.h>

class BossEnemy;

/// @brief ボスの行動。セレクター(成功で終了)を継承している。
class BossBehavior :
	public Selector
{
private:
	// ---------------
	// メンバ変数
	// ---------------

	BossEnemy* boss = nullptr;

	template<class T>
	struct MaxNow
	{
		T maxVal;
		T nowVal;
	};

	std::unique_ptr<Sequencer> nearTargetPhase;
	std::unique_ptr<Sequencer> farTargetPhase;

#pragma region 単発攻撃

	std::unique_ptr<Sequencer> singleShotPhase;

	struct SingleShotData
	{
		MaxNow<uint32_t> shotFrame{};
		MaxNow<uint32_t> count{};
		DirectX::XMFLOAT4 bulCol{};
	};

	std::unique_ptr<SingleShotData> singleShotData;

#pragma endregion 単発攻撃

#pragma region 扇形攻撃

	std::unique_ptr<Sequencer> fanShapePhase;

#pragma region 弾発射関係

	struct FanShotData
	{
		MaxNow<uint32_t> shotFrame{};
		MaxNow<uint32_t> count{};
		uint32_t shotNum{};
		DirectX::XMFLOAT4 bulCol{};
	};

	FanShotData fanShotData{};

#pragma endregion 弾発射関係

#pragma region 回転フェーズ

	struct RotationPhaseData
	{
		MaxNow<uint32_t> count = MaxNow<uint32_t>{ .maxVal = 120, .nowVal = 0 };
	};
	RotationPhaseData rotationPhaseData{};

#pragma endregion 回転フェーズ

#pragma endregion 扇形攻撃

#pragma region 自機を引き寄せる

	struct TornadoPhaseData
	{
		MaxNow<uint32_t> frame = MaxNow<uint32_t>{ .maxVal = 300, .nowVal = 0u };
		DirectX::XMFLOAT3 tornadoWorldPos{};
		float targetSpeed = 5.f;
	};
	TornadoPhaseData tornadoPhaseData{};

	std::unique_ptr<Sequencer> tornadoPhase;

#pragma endregion 自機を引き寄せる

private:
	// ---------------
	// priavteメンバ関数
	// ---------------

	NODE_RESULT phase_Rotation(const DirectX::XMFLOAT3& rotaMin,
							   const DirectX::XMFLOAT3& rotaMax = DirectX::XMFLOAT3(0, 360, 0));
	NODE_RESULT phase_fanShapeAttack();
	NODE_RESULT phase_singleShotAttack();
	NODE_RESULT phase_tornado();

	NODE_RESULT phase_setTornadoData();

public:
	BossBehavior(BossEnemy* boss);

	/// @brief bossはnullptrで初期化
	BossBehavior();

	/// @brief ボスをセット
	/// @param boss ボスのポインタ
	inline void setBoss(BossEnemy* boss) { this->boss = boss; }
};
