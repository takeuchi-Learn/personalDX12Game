#pragma once

#include "ObjModel.h"
#include "Object3d.h"
#include <DirectXMath.h>

class PlayerBullet {
	const bool needToDeleteObj = false;

	Object3d *obj = nullptr;

	DirectX::XMFLOAT3 pos{};
	bool posDirty = false;


public:
	bool alive = true;
	DirectX::XMFLOAT3 vel{};

	uint8_t life = 60;
	uint8_t age = 0;

public:
	PlayerBullet(Object3d *obj,
				 const DirectX::XMFLOAT3 &pos = { 0,0,0 });

	PlayerBullet(Camera *camera,
				 ObjModel *model,
				 const DirectX::XMFLOAT3 &pos = { 0,0,0 });

	~PlayerBullet();

	inline const DirectX::XMFLOAT3 &getPos() { return pos; }
	inline void setPos(const DirectX::XMFLOAT3 &pos) { this->pos = pos; posDirty = true; }

	inline const DirectX::XMFLOAT3 &getScale() { return obj->scale; }
	inline void setScale(const DirectX::XMFLOAT3 &scale) { obj->scale = scale; }

	void drawWithUpdate(Light *light);
};

