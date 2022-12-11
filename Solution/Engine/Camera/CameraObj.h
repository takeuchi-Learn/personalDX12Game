#pragma once
#include "GameObject/GameObj.h"
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

	DirectX::XMMATRIX matWorld{};

public:
	CameraObj(GameObj* parent);

	/// @param eye2TargetLen カメラ位置から注視点の距離
	inline void setEye2TargetLen(float eye2TargetLen) { this->eye2TargetLen = eye2TargetLen; }
	/// @return カメラ位置から注視点の距離
	inline float getEye2TargetLen() const { return eye2TargetLen; }

	/// @return ワールド行列
	inline const DirectX::XMMATRIX& getMatWorld() const { return matWorld; }

	/// @param parent 親オブジェクトのポインタ
	inline void setParentObj(GameObj* parent) { parentObj = parent; matWorldDirty = true; }
	/// @return 親オブジェクトのポインタ
	inline GameObj* getParentObj() const { return parentObj; }

	/// @param rotaDeg 親を基準とした回転
	inline void setRelativeRotaDeg(const DirectX::XMFLOAT3& rotaDeg) { relativeRotaDeg = rotaDeg; matWorldDirty = true; }
	/// @return 親を基準とした回転
	inline const DirectX::XMFLOAT3& getRelativeRotaDeg() const { return relativeRotaDeg; }

private:
	void updateMatWorld();

	void preUpdate() override;
};
