#pragma once

#include "GameObj.h"

/// @brief 自機の弾クラス
class PlayerBullet
	: public GameObj
{
	DirectX::XMFLOAT3 vel{};
	float speed = 1.f;

	GameObj* targetObjPt = nullptr;

public:

	uint16_t life = 180ui16;
	uint16_t age = 0ui16;

private:
	void additionalUpdate() override;

public:
	using GameObj::GameObj;

	inline GameObj* getTargetObjPt() const { return targetObjPt; }
	inline void setTargetObjPt(GameObj* target) { this->targetObjPt = target; }

	inline float getSpeed() const { return speed; }
	inline void setSpeed(float speed) { this->speed = speed; }

	// @return 毎秒進む値
	inline const DirectX::XMFLOAT3& getVel() const { return vel; }
	// @param vel 毎秒進む値
	inline void setVel(const DirectX::XMFLOAT3& vel) { this->vel = vel; }
};
