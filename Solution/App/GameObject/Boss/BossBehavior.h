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

	struct FanShotData
	{
		uint32_t interval{};
		uint32_t countMax{};
		uint32_t shotNum{};
		DirectX::XMFLOAT4 bulCol{};
	};

	FanShotData fanShotData{};

	uint32_t nowShotFrame = 0u;
	uint32_t shotCount = 0u;


#pragma endregion 弾発射関係

private:
	// ---------------
	// priavteメンバ関数
	// ---------------

	NODE_RESULT phase_approach();
	NODE_RESULT phase_leave();
	NODE_RESULT phase_attack();

public:
	BossBehavior(BossEnemy* boss);

	/// @brief bossはnullptrで初期化
	BossBehavior();

	/// @brief ボスをセット
	/// @param boss ボスのポインタ
	inline void setBoss(BossEnemy* boss) { this->boss = boss; }
};
