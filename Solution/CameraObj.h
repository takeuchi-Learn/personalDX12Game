#pragma once
#include "GameObj.h"
#include "Camera.h"
#include <memory>
class CameraObj
	: public Camera
{
private:
	GameObj* parentObj = nullptr;

	// 視点から注視点までの距離
	float eye2TargetLen = 150.f;
	// 親を基準とした回転
	DirectX::XMFLOAT3 relativeRotaDeg{};

	DirectX::XMFLOAT3 eye2TargetOffset = XMFLOAT3(0.f, 50.f, 50.f);

	bool matWorldDirty = false;

public:
	using Camera::Camera;

	CameraObj(GameObj* parent);

	inline void setParentObj(GameObj* parent) { parentObj = parent; matWorldDirty = true; }
	inline GameObj* getParentObj() const { return parentObj; }

	inline void setRelativeRotaDeg(const DirectX::XMFLOAT3& rotaDeg) { relativeRotaDeg = rotaDeg; matWorldDirty = true; }
	inline const DirectX::XMFLOAT3& getRelativeRotaDeg() const { return relativeRotaDeg; }

private:

	void preUpdate() override;
};
