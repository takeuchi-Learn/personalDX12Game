#include "CollisionMgr.h"
#include <Collision/Collision.h>
#include <DirectXMath.h>

using namespace DirectX;

using namespace CollisionShape;

void CollisionMgr::checkHitAll(const GroupAndHitProc& group1, const GroupAndHitProc& group2)
{
	auto& g1Col = colliders.at(group1.name);
	auto& g2Col = colliders.at(group2.name);

	for (auto& g1 : g1Col)
	{
		for (auto& g2 : g2Col)
		{
			if (Collision::CheckHit(Sphere(XMLoadFloat3(&g1.obj->calcWorldPos()), g1.colliderR),
									Sphere(XMLoadFloat3(&g2.obj->calcWorldPos()), g2.colliderR)))
			{
				group1.hitProc(g1.obj);
				group2.hitProc(g2.obj);
			}
		}
	}
}
