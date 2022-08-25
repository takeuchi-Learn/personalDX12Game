﻿#pragma once

#include "GameObj.h"
#include "EnemyBullet.h"

#include <functional>
#include <forward_list>

class Enemy
	: public GameObj {

	DirectX::XMFLOAT3 vel{};

	std::forward_list <std::unique_ptr<EnemyBullet>> bul;

	ObjModel *bulModel = nullptr;

	Camera *camera = nullptr;

	constexpr static const uint32_t shotFrameMax = 60U;
	uint32_t shotFrame = 0;

	GameObj *targetObjPt = nullptr;



	std::function<void()> phase;

	// 接近フェーズ
	void phase_Approach();
	// 離脱フェーズ
	void phase_Leave();



	void update() override;
	void additionalDraw(Light *light) override;



	DirectX::XMVECTOR Slerp(DirectX::XMVECTOR startVec, DirectX::XMVECTOR endVec, float raito);


	inline DirectX::XMFLOAT3 calcVel(const DirectX::XMFLOAT3 &targetPos,
									 const DirectX::XMFLOAT3 &nowPos,
									 float velScale) {
		DirectX::XMFLOAT3 velF3{
			targetPos.x - nowPos.x,
			targetPos.y - nowPos.y,
			targetPos.z - nowPos.z
		};

		const DirectX::XMVECTOR velVec =
			DirectX::XMVectorScale(
				DirectX::XMVector3Normalize(
					DirectX::XMLoadFloat3(&velF3)
				),
				velScale
			);

		DirectX::XMStoreFloat3(&velF3, velVec);
		return velF3;
	}

	inline DirectX::XMFLOAT2 calcRotationSyncVelRad(const DirectX::XMFLOAT3 &vel) {
		return DirectX::XMFLOAT2(DX12Base::getInstance()->near_atan2(-vel.y,
																	 sqrtf(vel.x * vel.x + vel.z * vel.z)),
								 DX12Base::getInstance()->near_atan2(vel.x, vel.z));
	}
	inline DirectX::XMFLOAT2 calcRotationSyncVelDeg(const DirectX::XMFLOAT3 &vel) {
		const DirectX::XMFLOAT2 rad = calcRotationSyncVelRad(vel);
		return DirectX::XMFLOAT2(DirectX::XMConvertToDegrees(rad.x), DirectX::XMConvertToDegrees(rad.y));
	}

public:
	using GameObj::GameObj;

	Enemy(Camera *camera,
		  ObjModel *model,
		  ObjModel *bulModel,
		  const DirectX::XMFLOAT3 &pos = { 0,0,0 });

	inline void setTargetObj(GameObj *targetObj) { this->targetObjPt = targetObj; }

	inline auto &getBulList() { return bul; }

	inline bool bulEmpty() const { return bul.empty(); }

	// @param vel 毎秒進む値
	void shot(const DirectX::XMFLOAT3 &targetPos,
			  float vel = 1.f,
			  float bulScale = 10.f);

	// @return 毎秒進む値
	inline const DirectX::XMFLOAT3 &getVel() { return vel; }
	// @param vel 毎秒進む値
	inline void setVel(const DirectX::XMFLOAT3 &vel) { this->vel = vel; }

};

