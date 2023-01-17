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

/// @brief ボスの行動
class BossBehavior
{
public:

	/// @brief ボスのフェーズ
	enum class PHASE : uint8_t
	{
		APPROACH,	/// 接近
		LEAVE,		/// 離脱
		ATTACK,		/// 攻撃
	};

private:
	// ---------------
	// メンバ変数
	// ---------------

	BossEnemy* boss;
	std::unique_ptr<Selector> rootNode;

	PHASE phase;

#pragma region 弾発射関係

	static inline constexpr uint32_t shotInterval = 60u;
	uint32_t nowShotFrame = 0u;
	static inline constexpr uint32_t shotCountMax = 15u;
	uint32_t shotCount = 0u;
	static inline constexpr uint32_t shotEnemyNum = 15;

	static inline constexpr DirectX::XMFLOAT4 bulCol =
		DirectX::XMFLOAT4(1, 0.25f, 0.125f, 1.f);

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

	/// @brief 現在のフェーズを取得
	/// @return 現在のフェーズ
	inline PHASE getPhase() const { return phase; }

	/// @brief ルートノードを実行実行
	/// @return 実行結果
	inline NODE_RESULT run() { return rootNode->run(); }
};
