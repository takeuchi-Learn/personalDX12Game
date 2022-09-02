#pragma once
#include <DirectXMath.h>
#include <memory>

#include "ObjSet.h"
#include "PlayerBullet.h"
#include <forward_list>

#include "GameObj.h"

class Player
	: public GameObj {
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMVECTOR = DirectX::XMVECTOR;

	std::forward_list<PlayerBullet> bul;

	DirectX::XMFLOAT2 aim2DPos{};

	Object3d *shotTargetObjPt = nullptr;

public:
	Player(Camera *camera,
		   ObjModel *model,
		   const DirectX::XMFLOAT3 &pos = { 0.f,0.f,0.f });

	XMVECTOR getLookVec(float len = 1.f);

	inline bool shotTargetIsEmpty() const { return shotTargetObjPt != nullptr; }
	inline void setShotTarget(Object3d *targetPt) { shotTargetObjPt = targetPt; }

	inline auto &getBulArr() { return bul; }

	inline auto getMatWorld() const { return obj->getMatWorld(); }

	inline void setParent(GameObj *parent) { obj->parent = parent->getObj(); }

	inline const DirectX::XMFLOAT2 &getAim2DPos() const { return aim2DPos; }
	inline void setAim2DPos(const DirectX::XMFLOAT2 &screenPos) { aim2DPos = screenPos; }

	/// <summary>
	/// 視線方向に前進
	/// </summary>
	/// <param name="moveVel">移動量</param>
	/// <param name="moveYFlag">Y方向に移動するか</param>
	void moveForward(float moveVel, bool moveYFlag = false);

	/// <summary>
	/// 右に移動。前進処理のベクトルを右に90度傾けた移動。
	/// </summary>
	/// <param name="moveVel">移動量</param>
	/// <param name="moveYFlag">Y方向に移動するか</param>
	void moveRight(float moveVel, bool moveYFlag = false);

	/// <summary>
	/// 上に移動。前進処理のベクトルを上に90度傾けた移動。
	/// </summary>
	/// <param name="moveVel">移動量</param>
	void moveUp(float moveVel);

	// @param vel 毎秒進む値
	void shot(Camera *camera,
			  ObjModel *model,
			  float speed = 1.f,
			  float bulScale = 10.f);

	void additionalUpdate() override;
	void additionalDraw(Light *light) override;
};

