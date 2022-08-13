#pragma once
#include <DirectXMath.h>
#include <memory>

#include "ObjSet.h"

class Player {
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMVECTOR = DirectX::XMVECTOR;

	std::unique_ptr<ObjSet> obj;

public:
	Player(Camera *camera,
		   const std::string &dirPath,
		   const std::string &name,
		   bool smoothing = false)
		: obj(new ObjSet(camera, dirPath, name, smoothing)) {
	}

	inline const XMFLOAT3 &getPos() { return obj->getPos(); }
	inline void setPos(const XMFLOAT3 &pos) { obj->setPos(pos); }

	inline const XMFLOAT3 &getRotation() { return obj->getRotation(); }
	inline void setRotation(const XMFLOAT3 &rota) { obj->setRotation(rota); }

	inline const XMFLOAT3 &getScale() { return obj->getScale(); }
	inline void setScale(const XMFLOAT3 &scale) { obj->setScale(scale); }

	XMVECTOR getLookVec(float len = 1.f);

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

	void drawWithUpdate(Light *light);
};

