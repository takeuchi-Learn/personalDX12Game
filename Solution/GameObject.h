#pragma once

#include "CollisionShape.h"
#include <memory>
#include "Object3d.h"
#include "ObjModel.h"
#include "Light.h"
#include <DirectXMath.h>

class GameObject {
	std::unique_ptr<CollisionShape> collisionShape;
	std::unique_ptr<Object3d> obj3d;
	std::unique_ptr<ObjModel> model;

public:
	bool alive = true;

public:
	inline DirectX::XMFLOAT3 getPos() { return obj3d->position; }
	inline void setPos(const DirectX::XMFLOAT3 &pos) { obj3d->position = pos; }

	inline DirectX::XMFLOAT3 getScale() { return obj3d->scale; }
	inline void setScale(const DirectX::XMFLOAT3 &scale) { obj3d->scale = scale; }

	GameObject(std::unique_ptr<CollisionShape> &&col,
			   std::unique_ptr<Object3d> &&obj3d,
			   std::unique_ptr<ObjModel> &&model);

	void drawWithUpdate(Light *light);
};

