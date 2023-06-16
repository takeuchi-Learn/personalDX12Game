#pragma once

#include "BaseEnemy.h"
#include "EnemyBullet.h"

#include <functional>
#include <forward_list>

/// @brief レールシューティングの通常敵クラス
class NormalEnemy
	: public BaseEnemy
{
	DirectX::XMFLOAT3 vel{};

	std::forward_list <std::unique_ptr<EnemyBullet>> bul;

	ObjModel* bulModel = nullptr;

	Camera* camera = nullptr;

	constexpr static const uint32_t shotFrameMax = 240U;
	uint32_t shotFrame = 0;

	GameObj* targetObjPt = nullptr;

	uint16_t nowFrame = 0u;

	// 接近フェーズ
	void phase_Approach();
	// 離脱フェーズ
	void phase_Leave();

	void afterUpdate() override;
	void additionalDraw(Light* light) override;

public:
	NormalEnemy(Camera* camera,
				ObjModel* model,
				ObjModel* bulModel,
				const DirectX::XMFLOAT3& pos = { 0,0,0 });

	inline void setTargetObj(GameObj* targetObj) { this->targetObjPt = targetObj; }

	inline auto& getBulList() { return bul; }

	inline bool bulEmpty() const { return bul.empty(); }

	// @param vel 毎秒進む値
	void shot(const DirectX::XMFLOAT3& targetPos,
			  float vel = 1.f,
			  float bulScale = 10.f);

	// @return 進む値
	inline const DirectX::XMFLOAT3& getVel() { return vel; }
	// @param vel 進む値
	inline void setVel(const DirectX::XMFLOAT3& vel) { this->vel = vel; }

	inline void chansePhase_Leave(const DirectX::XMFLOAT3& vel)
	{
		setVel(vel);
		setPhase(std::bind(&NormalEnemy::phase_Leave, this));
	}
};
