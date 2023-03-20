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

/// @brief ボスの行動。シーケンサー(失敗で終了)を継承している。
class BossBehavior :
	public Sequencer
{
private:
	// ---------------
	// メンバ変数
	// ---------------

	BossEnemy* boss = nullptr;

#pragma region 弾発射関係

	template<class T>
	struct MaxNow
	{
		T MaxVal;
		T nowVal;
	};

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
		uint32_t countMax = 120;
		DirectX::XMFLOAT3 rotaMax = DirectX::XMFLOAT3(0, 360, 0);
	};
	RotationPhaseData rotationPhaseData{};
	uint32_t rotaPhaseNowCount = 0;

#pragma endregion 回転フェーズ


private:
	// ---------------
	// priavteメンバ関数
	// ---------------

	NODE_RESULT phase_Rotation();
	NODE_RESULT phase_fanShapeAttack();

public:
	BossBehavior(BossEnemy* boss);

	/// @brief bossはnullptrで初期化
	BossBehavior();

	/// @brief ボスをセット
	/// @param boss ボスのポインタ
	inline void setBoss(BossEnemy* boss) { this->boss = boss; }
};
