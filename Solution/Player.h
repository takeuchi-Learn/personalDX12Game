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

	std::vector<std::unique_ptr<Object3d>> aimObj;
	const float aimObjLen;

	bool showAimObjFlag;

public:
	Player(Camera *camera,
		   ObjModel *model,
		   const DirectX::XMFLOAT3 &pos = { 0.f,0.f,0.f });

	XMVECTOR getLookVec(float len = 1.f);

	inline auto &getBulArr() { return bul; }

	inline auto getMatWorld() const { return obj->getMatWorld(); }

	inline void setParent(GameObj *parent) { obj->parent = parent->getObj(); }

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

	// @param vel 毎秒進む値
	void shot(Camera *camera,
			  ObjModel *model,
			  float speed = 1.f,
			  float bulScale = 10.f);

	void additionalUpdate() override;
	void additionalDraw(Light *light) override;
};

