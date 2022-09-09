#pragma once

#include "ObjModel.h"
#include "Object3d.h"
#include <DirectXMath.h>
#include <memory>

class GameObj
{
protected:
	std::unique_ptr<Object3d> obj;

	bool alive = true;

	virtual void additionalUpdate() {};
	virtual void additionalDraw(Light* light) {}

public:
	inline static DirectX::XMFLOAT2 calcRotationSyncVelRad(const DirectX::XMFLOAT3& vel)
	{
		return DirectX::XMFLOAT2(DX12Base::getInstance()->near_atan2(-vel.y,
																	 sqrtf(vel.x * vel.x + vel.z * vel.z)),
								 DX12Base::getInstance()->near_atan2(vel.x, vel.z));
	}
	inline static DirectX::XMFLOAT2 calcRotationSyncVelDeg(const DirectX::XMFLOAT3& vel)
	{
		const DirectX::XMFLOAT2 rad = calcRotationSyncVelRad(vel);
		return DirectX::XMFLOAT2(DirectX::XMConvertToDegrees(rad.x), DirectX::XMConvertToDegrees(rad.y));
	}

	inline void setParent(Object3d* parent) { obj->parent = parent; }

	inline bool getAlive() const { return alive; }
	// aliveをfalseにする
	inline void kill() { alive = false; }

	inline Object3d* getObj() const { return obj.get(); }

	inline void setPos(const DirectX::XMFLOAT3& pos) { obj->position = pos; }
	inline const DirectX::XMFLOAT3& getPos() const { return obj->position; }

	inline void setScaleF3(const DirectX::XMFLOAT3& scale) { obj->scale = scale; }
	inline void setScale(float scale) { obj->scale = DirectX::XMFLOAT3(scale, scale, scale); }
	inline const DirectX::XMFLOAT3& getScaleF3() const { return obj->scale; }
	inline float getScale() const { return obj->scale.x; }

	inline const DirectX::XMFLOAT3& getRotation() const { return obj->rotation; }
	inline void setRotation(const DirectX::XMFLOAT3& rota) { obj->rotation = rota; }

	inline const DirectX::XMMATRIX& getMatWorld() const { return obj->getMatWorld(); }

	GameObj(Camera* camera,
			ObjModel* model,
			const DirectX::XMFLOAT3& pos = { 0,0,0 });

	// drawWithUpdate関数の頭で呼ばれる
	void update();

	// drawWithUpdate関数で呼ばれる
	void draw(Light* light);

	void drawWithUpdate(Light* light);
};
