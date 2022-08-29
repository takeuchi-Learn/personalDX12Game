#pragma once
#include "GameObj.h"
#include "Camera.h"
#include <memory>
class CameraObj
	: public Camera {
private:
	GameObj *parentObj = nullptr;

	// 親を基準とした回転
	DirectX::XMFLOAT3 relativeRotaDeg{};

	bool matWorldDirty = false;

public:
	using Camera::Camera;

	CameraObj(GameObj *parent);

	inline void setParentObj(GameObj *parent) { parentObj = parent; matWorldDirty = true; }
	inline GameObj *getParentObj() const { return parentObj; }

	inline void setRelativeRotaDeg(const DirectX::XMFLOAT3 &rotaDeg) { relativeRotaDeg = rotaDeg; matWorldDirty = true; }
	inline const DirectX::XMFLOAT3 &getRelativeRotaDeg() const { return relativeRotaDeg; }

private:

	void preUpdate() override;

};

