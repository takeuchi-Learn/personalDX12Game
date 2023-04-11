/*****************************************************************//**
 * \file   CollisionMgr.h
 * \brief  衝突判定をするクラス。現状では球体のみ対応している。
 *********************************************************************/

#pragma once
#include <functional>
#include <forward_list>
#include <GameObject/GameObj.h>

 /// @brief 衝突判定をするクラス
class CollisionMgr
{
public:
	struct ColliderType
	{
		GameObj* obj = nullptr;
		float colliderR = 1.f;

		inline static ColliderType create(GameObj* obj)
		{
			return ColliderType{ .obj = obj, .colliderR = obj->getScaleF3().z };
		}
	};

	using GroupType = std::forward_list<ColliderType>;

	struct ColliderSet
	{
		/// @brief コライダーグループ
		GroupType group;

		/// @brief 衝突したときに行う処理
		std::function<void(GameObj*)> hitProc;
	};

	static void checkHitAll(const ColliderSet& collider1,
							const ColliderSet& collider2);
};

