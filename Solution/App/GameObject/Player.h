#pragma once
#include <DirectXMath.h>
#include <memory>

#include "PlayerBullet.h"
#include <forward_list>
#include <set>

#include "GameObj.h"

/// @brief 自機クラス
class Player
	: public GameObj
{
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMVECTOR = DirectX::XMVECTOR;

	std::forward_list<PlayerBullet> bul;
	uint16_t bulLife = 180ui16;

	DirectX::XMFLOAT2 aim2DPos{};

	GameObj* shotTargetObjPt;

	uint16_t hp;

public:
	Player(Camera* camera,
		   ObjModel* model,
		   const DirectX::XMFLOAT3& pos = { 0.f,0.f,0.f },
		   uint16_t hp = 24u);

	inline uint16_t getBulLife() const { return bulLife; }
	inline void setBulLife(uint16_t bulLife) { this->bulLife = bulLife; }

	uint16_t getHp() const { return hp; }
	void setHp(uint16_t hp) { this->hp = hp; }
	bool damage(uint16_t damegeNum, bool killFlag = true);

	XMVECTOR getLookVec(float len = 1.f);

	inline const GameObj* getShotTarget() const { return shotTargetObjPt; }
	inline void setShotTarget(GameObj* targetPt) { shotTargetObjPt = targetPt; }

	inline auto& getBulArr() { return bul; }

	inline auto getMatWorld() const { return obj->getMatWorld(); }

	inline void setParent(GameObj* parent) { obj->parent = parent->getObj(); }

	inline const DirectX::XMFLOAT2& getAim2DPos() const { return aim2DPos; }
	inline void setAim2DPos(const DirectX::XMFLOAT2& screenPos) { aim2DPos = screenPos; }

	/// @brief 視線方向に前進
	/// @param moveVel 移動量
	/// @param moveYFlag Y方向に移動するか
	void moveForward(float moveVel, bool moveYFlag = false);

	/// @brief 右に移動。前進処理のベクトルを右に90度傾けた移動。
	/// @param moveVel 移動量
	/// @param moveYFlag Y方向に移動するか
	void moveRight(float moveVel, bool moveYFlag = false);

	/// @brief 上に移動。前進処理のベクトルを上に90度傾けた移動。
	/// @param moveVel 移動量
	void moveUp(float moveVel);

	/// @brief 弾発射
	/// @param camera カメラオブジェクトのポインタ
	/// @param model 弾のモデル
	/// @param speed 速度
	/// @param bulScale 弾の大きさ
	void shot(Camera* camera,
			  ObjModel* model,
			  float speed = 1.f,
			  float bulScale = 10.f);

	void additionalUpdate() override;
	void additionalDraw(Light* light) override;
};
