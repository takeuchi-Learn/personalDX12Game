﻿#pragma once
#include <DirectXMath.h>
#include <memory>

#include "PlayerBullet.h"
#include <forward_list>

#include "GameObj.h"

class Player
	: public GameObj
{
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMVECTOR = DirectX::XMVECTOR;

	std::forward_list<PlayerBullet> bul;

	DirectX::XMFLOAT2 aim2DPos{};

	Object3d* shotTargetObjPt = nullptr;

public:
	Player(Camera* camera,
		   ObjModel* model,
		   const DirectX::XMFLOAT3& pos = { 0.f,0.f,0.f });

	XMVECTOR getLookVec(float len = 1.f);

	inline bool shotTargetIsEmpty() const { return shotTargetObjPt != nullptr; }
	inline void setShotTarget(Object3d* targetPt) { shotTargetObjPt = targetPt; }

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