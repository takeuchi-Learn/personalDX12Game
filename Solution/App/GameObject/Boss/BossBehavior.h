/*****************************************************************//**
 * \file   BossBehavior.h
 * \brief  ボスの行動
 *********************************************************************/

#pragma once

#include <BehaviorTree/Selector.h>
#include <BehaviorTree/Sequencer.h>
#include <memory>
#include <GameObject/Boss/BossEnemy.h>

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
