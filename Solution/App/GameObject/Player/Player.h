#pragma once
#include <DirectXMath.h>
#include <memory>

#include "PlayerBullet.h"
#include <forward_list>
#include <vector>

#include <GameObject/GameObj.h>
#include <3D/ParticleMgr.h>

/// @brief 自機クラス
class Player
	: public GameObj
{
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMVECTOR = DirectX::XMVECTOR;

	std::forward_list<PlayerBullet> bul;
	uint16_t bulLife = 180ui16;

	DirectX::XMFLOAT2 aim2DPos{};

	std::vector<std::weak_ptr<GameObj>> shotTargetObjPt;

	std::shared_ptr<ParticleMgr> bulParticle;

	uint16_t hp = 1ui16;

	float bulHomingRaito = 0.05f;

public:
	Player(Camera* camera,
		   ObjModel* model,
		   const DirectX::XMFLOAT3& pos = { 0,0,0 });

	inline float getBulHomingRaito() const { return bulHomingRaito; }
	inline void setBulHomingRaito(float raito) { bulHomingRaito = raito; }

	inline uint16_t getBulLife() const { return bulLife; }
	inline void setBulLife(uint16_t bulLife) { this->bulLife = bulLife; }

	uint16_t getHp() const { return hp; }
	void setHp(uint16_t hp) { this->hp = hp; }
	bool damage(uint16_t damegeNum, bool killFlag = true);

	XMVECTOR getLookVec(float len = 1.f);

	inline const auto& getShotTarget() const { return shotTargetObjPt; }
	/// @brief 重複無しで攻撃対象を追加
	/// @return 追加したらtrue
	inline bool addShotTarget(std::weak_ptr<GameObj> targetPt)
	{
		if (targetPt.expired()) { return false; }
		auto target = targetPt.lock();

		for (auto& i : shotTargetObjPt)
		{
			if (i.expired()) { continue; }

			if (target == i.lock())
			{
				return false;
			}
		}
		shotTargetObjPt.emplace_back(target);
		return true;
	}
	inline void deleteShotTarget() { shotTargetObjPt.clear(); }

	inline auto& getBulArr() { return bul; }

	inline auto getMatWorld() const { return obj->getMatWorld(); }

	inline void setParent(GameObj* parent) { obj->parent = parent->getObj(); }

	inline const DirectX::XMFLOAT2& getAim2DPos() const { return aim2DPos; }
	inline void setAim2DPos(const DirectX::XMFLOAT2& screenPos) { aim2DPos = screenPos; }

	inline void drawWithUpdateBulParticle() { bulParticle->drawWithUpdate(); }

	/// @brief 弾発射
	/// @param camera カメラオブジェクトのポインタ
	/// @param model 弾のモデル
	/// @param speed 速度
	/// @param bulScale 弾の大きさ
	/// @return	ターゲットを設定したかどうか
	bool shotAll(Camera* camera,
			  ObjModel* model,
			  float speed = 1.f,
			  float bulScale = 10.f);

	void additionalUpdate() override;
	void additionalDraw(Light* light) override;
};
