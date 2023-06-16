#include "CollisionMgr.h"
#include <Collision/CollisionShape.h>
#include <Collision/Collision.h>

void CollisionMgr::checkHitAll(const ColliderSet& collider1,
							   const ColliderSet& collider2)
{
	if (collider1.group.empty() || collider2.group.empty()) { return; }

	for (auto& g1 : collider1.group)
	{
		if (!g1.obj) { continue; }
		if (!g1.obj->getAlive()) { continue; }

		for (auto& g2 : collider2.group)
		{
			if (!g2.obj) { continue; }
			if (!g2.obj->getAlive()) { continue; }

			if (Collision::CheckHit(CollisionShape::Sphere(XMLoadFloat3(&g1.obj->calcWorldPos()), g1.colliderR),
									CollisionShape::Sphere(XMLoadFloat3(&g2.obj->calcWorldPos()), g2.colliderR)))
			{
				collider1.hitProc(g1.obj);
				collider2.hitProc(g2.obj);
			}
		}
	}
}
