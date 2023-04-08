/*****************************************************************//**
 * \file   CollisionMgr.h
 * \brief  衝突判定をするクラス。現状では球体のみ対応している。
 *********************************************************************/

#pragma once
#include <unordered_map>
#include <functional>
#include <forward_list>
#include <string>
#include <Collision/CollisionShape.h>
#include <GameObject/GameObj.h>

 /// @brief 衝突判定をするクラス
class CollisionMgr
{
public:
	struct ColliderType
	{
		GameObj* obj = nullptr;
		float colliderR = 1.f;
	};

	using GroupType = std::forward_list<ColliderType>;

	struct ColliderSet
	{
		GroupType group;
		std::function<void(GameObj*)> hitProc;
	};

	void checkHitAll(const ColliderSet& collider1,
					 const ColliderSet& collider2);
};

