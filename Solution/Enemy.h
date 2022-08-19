#pragma once

#include "GameObj.h"

#include <functional>

class Enemy
	: public GameObj {

	DirectX::XMFLOAT3 vel{};



	std::function<void()> phase;

	// 接近フェーズ
	void phase_Approach();
	// 離脱フェーズ
	void phase_Leave();



	void update(Light *light) override;
public:
	using GameObj::GameObj;

	Enemy(Camera *camera,
		  ObjModel *model,
		  const DirectX::XMFLOAT3 &pos = { 0,0,0 });

	// @return 毎秒進む値
	inline const DirectX::XMFLOAT3 &getVel() { return vel; }
	// @param vel 毎秒進む値
	inline void setVel(const DirectX::XMFLOAT3 &vel) { this->vel = vel; }

};

