#include "ReticleSphere.h"

#include <Collision/Collision.h>
#include <Camera/Camera.h>

using namespace DirectX;

ReticleSphere::ReticleSphere(const Camera* camera,
							 const XMFLOAT2& screenPos,
							 float distance,
							 float reticleR)
{
	const XMVECTOR center = camera->screenPos2WorldPosVec(XMFLOAT3(screenPos.x, screenPos.y, distance));
	const XMVECTOR right = camera->screenPos2WorldPosVec(XMFLOAT3(screenPos.x + reticleR, screenPos.y, distance));

	this->center = center;
	this->radius = Collision::vecLength(XMVectorSubtract(center, right));
}