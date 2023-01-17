/*****************************************************************//**
 * \file   BaseEnemy.h
 * \brief  敵基底クラス
 *********************************************************************/

#pragma once
#include "GameObj.h"
#include <functional>

 /// @brief 敵基底クラス
class BaseEnemy
	: public GameObj
{
protected:
	uint16_t hp;

	std::function<void()> phase;

	DirectX::XMFLOAT3 vel{};

	Camera* camera = nullptr;

	uint32_t life = UINT32_MAX;

	virtual void beforeUpdate() {}
	virtual void afterUpdate() {}

	void additionalUpdate() override;

	inline void movePos(const DirectX::XMFLOAT3& vel)
	{
		obj->position.x += vel.x;
		obj->position.y += vel.y;
		obj->position.z += vel.z;
	}

public:
	BaseEnemy(Camera* camera,
			  ObjModel* model,
			  const DirectX::XMFLOAT3& pos = { 0,0,0 },
			  uint16_t hp = 1ui16);

	inline void setLife(uint32_t life) { this->life = life; }
	inline uint32_t getLife() const { return life; }

	inline void setHp(uint16_t hp) { this->hp = hp; }
	inline uint16_t getHp() const { return hp; }

	inline void setVel(const DirectX::XMFLOAT3& vel) { this->vel = vel; }
	inline const DirectX::XMFLOAT3& getVel() const { return vel; }

	/// @brief ダメージを与える
	/// @param damegeNum 与えるダメージ数
	/// @param killFlag hpが0になったらkillするかどうか(trueでkillする)
	/// @return 倒したかどうか(倒したらtrue)
	bool damage(uint16_t damegeNum, bool killFlag = true);

	inline void setPhase(const std::function<void()>& _phase) { this->phase = _phase; }
};
