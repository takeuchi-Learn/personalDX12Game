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

	inline void setVel(const DirectX::XMFLOAT3& vel) { this->vel = vel; }
	inline const DirectX::XMFLOAT3& getVel() const { return vel; }

	inline void setPhase(const std::function<void()>& _phase) { this->phase = _phase; }
};
