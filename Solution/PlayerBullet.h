#pragma once

#include "ObjModel.h"
#include "Object3d.h"
#include <DirectXMath.h>

class PlayerBullet {
	const bool needToDeleteObj = false;

	Object3d *obj = nullptr;

	DirectX::XMFLOAT3 pos{};
	bool posDirty = false;

	DirectX::XMFLOAT3 vel{};

public:
	bool alive = true;

	uint8_t life = 60;
	uint8_t age = 0;

public:
	PlayerBullet(Object3d *obj,
				 const DirectX::XMFLOAT3 &pos = { 0,0,0 });

	PlayerBullet(Camera *camera,
				 ObjModel *model,
				 const DirectX::XMFLOAT3 &pos = { 0,0,0 });

	~PlayerBullet();

	inline const DirectX::XMFLOAT3 &getVel() { return vel; }
	inline void setVel(const DirectX::XMFLOAT3 &vel) { this->vel = vel; }

	inline const DirectX::XMFLOAT3 &getPos() { return pos; }
	inline void setPos(const DirectX::XMFLOAT3 &pos) { this->pos = pos; posDirty = true; }

	inline const DirectX::XMFLOAT3 &getScale() { return obj->scale; }
	inline void setScale(float scale) { obj->scale = DirectX::XMFLOAT3(scale, scale, scale); }

	void drawWithUpdate(Light *light);
};

