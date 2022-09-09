#pragma once

#include "GameObj.h"

class PlayerBullet
	: public GameObj
{
	DirectX::XMFLOAT3 vel{};

public:

	uint8_t life = 180;
	uint8_t age = 0;

private:
	void additionalUpdate() override;

public:
	using GameObj::GameObj;

	// @return 毎秒進む値
	inline const DirectX::XMFLOAT3& getVel() { return vel; }
	// @param vel 毎秒進む値
	inline void setVel(const DirectX::XMFLOAT3& vel) { this->vel = vel; }
};
