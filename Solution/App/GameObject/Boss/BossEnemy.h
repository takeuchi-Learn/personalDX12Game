/*****************************************************************//**
 * \file   BossEnemy.h
 * \brief  ボス敵クラス
 *********************************************************************/

#pragma once
#include <GameObject/BaseEnemy.h>
#include <forward_list>
#include <GameObject/Boss/BossBehavior.h>

 /// @brief ボス敵クラス
class BossEnemy :
	public BaseEnemy
{
	friend class BossBehavior;

	std::unique_ptr<BossBehavior> bossBehavior;

	// 攻撃対象へのポインタ
	GameObj* targetObj = nullptr;

	// 移動速度
	float moveSpeed = 2.f;

	/// @brief 原点からボスの攻撃対象を向くベクトルを算出
	/// @param me 原点
	/// @param moveYFlag Y方向に移動するかどうか
	/// @return 原点->ボスのベクトル
	DirectX::XMVECTOR calcVelVec(GameObj* me, bool moveYFlag = false);

	void moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec);

	void afterUpdate() override;
	void additionalDraw(Light* light) override;

public:
	BossEnemy(Camera* camera,
			  ObjModel* model,
			  const DirectX::XMFLOAT3& pos = { 0,0,0 },
			  uint16_t hp = 1ui16);

	/// @brief 攻撃対象を設定
	/// @param obj 攻撃対象オブジェクトのポインタ
	inline void setTargetObj(GameObj* obj) { targetObj = obj; }
	/// @brief 攻撃対象を取得
	/// @return 攻撃対象のポインタ
	inline GameObj* getTargetObj() { return targetObj; }

#pragma region 弾関係

private:
	ObjModel* smallEnemyModel = nullptr;
	std::forward_list<std::unique_ptr<BaseEnemy>> smallEnemy;
	float smallEnemyMoveSpeed = 2.f;

	static const inline uint32_t bulLife = 900u;

public:
	/// @brief 弾として出された小さい敵の数を算出
	/// @return 小さい敵の数
	inline size_t calcSmallEnemyNum() const { return std::distance(smallEnemy.begin(), smallEnemy.end()); }

	inline const auto& getSmallEnemyList() const { return smallEnemy; }

	inline ObjModel* getSmallEnemyModel() { return smallEnemyModel; }
	inline void setSmallEnemyModel(ObjModel* model) { smallEnemyModel = model; }

	/// @brief 小さい敵を弾として出す
	void addSmallEnemyHoming();

	void addSmallEnemy(const DirectX::XMVECTOR& direction,
					   const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(1.f,
																		  1.f,
																		  1.f,
																		  1.f));

#pragma endregion 弾関係
};
