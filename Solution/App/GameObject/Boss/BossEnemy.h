﻿/*****************************************************************//**
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

	float maxTargerDistance = 1500.f;

	/// @brief 原点からボスの攻撃対象を向くベクトルを算出
	/// @param me 原点
	/// @param moveYFlag Y方向に移動するかどうか
	/// @return 原点->ボスのベクトル
	DirectX::XMVECTOR calcVelVec(GameObj* me, bool moveYFlag = false);

	/// @return 移動速度
	void move(float moveSpeed, const DirectX::XMVECTOR& velVec, DirectX::XMFLOAT3* velBuf = nullptr);

	/// @brief move()した後に移動方向を向く
	/// @return 移動速度
	void moveAndRota(float moveSpeed, const DirectX::XMVECTOR& velVec, DirectX::XMFLOAT3* velBuf = nullptr);

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

	inline float getMaxTargetDistance() const { return maxTargerDistance; }

	float calcTargetDistance();

#pragma region 弾関係

private:
	class Bul :
		public BaseEnemy
	{
		uint32_t life = UINT32_MAX;

	public:
		using BaseEnemy::BaseEnemy;

		inline uint32_t getLife() const { return life; }
		inline void setLife(uint32_t life) { this->life = life; }

		void afterUpdate() override;
	};

	std::forward_list<std::shared_ptr<Bul>> bul;
	ObjModel* bulModel = nullptr;

	static const inline uint32_t bulLife = 900u;

public:
	/// @brief 弾として出された小さい敵の数を算出
	/// @return 小さい敵の数
	inline size_t calcBulNum() const { return std::distance(bul.begin(), bul.end()); }

	inline const auto& getBulList() const { return bul; }

	inline ObjModel* getBulModel() { return bulModel; }
	inline void setBulModel(ObjModel* model) { bulModel = model; }

	/// @brief 小さい敵を弾として出す
	void addBulHoming(const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f),
					  float moveSpeed = 2.f);

	void addBul(const DirectX::XMVECTOR& direction,
				const DirectX::XMFLOAT3& scale = DirectX::XMFLOAT3(10, 10, 10),
				const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f),
				float moveSpeed = 2.f);

#pragma endregion 弾関係
};
