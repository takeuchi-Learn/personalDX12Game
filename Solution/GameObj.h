#pragma once

#include "ObjModel.h"
#include "Object3d.h"
#include <DirectXMath.h>
#include <memory>

class GameObj {

protected:
	std::unique_ptr<Object3d> obj;

	bool alive = true;

	// drawWithUpdate関数の頭で呼ばれる
	virtual void update(Light *light = nullptr) {};

public:
	inline bool getAlive() const { return alive; }
	// aliveをfalseにする
	inline void kill() { alive = false; }

	inline void setPos(const DirectX::XMFLOAT3 &pos) { obj->position = pos; }
	inline const DirectX::XMFLOAT3 &getPos() const { return obj->position; }

	inline void setScaleF3(const DirectX::XMFLOAT3 &scale) { obj->scale = scale; }
	inline void setScale(float scale) { obj->scale = DirectX::XMFLOAT3(scale, scale, scale); }
	inline const DirectX::XMFLOAT3 &getScaleF3() const { return obj->scale; }
	inline float getScale() const { return obj->scale.x; }

	inline const DirectX::XMFLOAT3 &getRotation() const { return obj->rotation; }
	inline void setRotation(const DirectX::XMFLOAT3 &rota) { obj->rotation = rota; }

	inline const DirectX::XMMATRIX &getMatWorld() const { return obj->getMatWorld(); }

	GameObj(Camera *camera,
			ObjModel *model,
			const DirectX::XMFLOAT3 &pos = { 0,0,0 });

	void drawWithUpdate(Light *light);
};

