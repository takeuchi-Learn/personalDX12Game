#pragma once

#include <Collision/CollisionShape.h>

#include <DirectXMath.h>

class Camera;

class ReticleSphere :
	public CollisionShape::Sphere
{
public:
	/// @param camera カメラ
	/// @param screenPos スクリーン座標での位置
	/// @param distance 生成する球とカメラの距離
	/// @param reticleR 照準画像の内接円の半径
	ReticleSphere(const Camera* camera, const DirectX::XMFLOAT2& screenPos, float distance, float reticleR);
};
