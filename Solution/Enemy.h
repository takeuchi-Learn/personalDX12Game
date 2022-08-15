#pragma once

#include "GameObj.h"

class Enemy
	: public GameObj {

	DirectX::XMFLOAT3 vel{};

	void update(Light *light) override;
public:
	using GameObj::GameObj;

	// @return 毎秒進む値
	inline const DirectX::XMFLOAT3 &getVel() { return vel; }
	// @param vel 毎秒進む値
	inline void setVel(const DirectX::XMFLOAT3 &vel) { this->vel = vel; }

};

