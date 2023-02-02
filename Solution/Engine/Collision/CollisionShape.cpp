#include "CollisionShape.h"

using namespace DirectX;

void Triangle::ComputeNormal()
{
	//外積により。2辺に垂直なベクトルを算出
	normal = XMVector3Cross(p1 - p0, p2 - p0);
	normal = XMVector3Normalize(normal);
}