#pragma once

#include "GameObj.h"

class EnemyBullet
	: public GameObj
{
	DirectX::XMFLOAT3 vel{ 0,0,-2 };

public:

	uint16_t life = 720;
	uint16_t age = 0;

private:
	void additionalUpdate() override;

public:
	using GameObj::GameObj;

	// @return 毎秒進む値
	inline const DirectX::XMFLOAT3& getVel() { return vel; }
	// @param vel 毎秒進む値
	inline void setVel(const DirectX::XMFLOAT3& vel) { this->vel = vel; }
};
