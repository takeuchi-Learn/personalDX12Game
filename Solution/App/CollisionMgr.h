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
private:
	struct ColliderType
	{
		GameObj* obj = nullptr;
		float colliderR = 1.f;
	};

	using GroupType = std::forward_list<ColliderType>;

	std::unordered_map<std::string, GroupType> colliders;

public:
	/// @brief コライダーのグループを追加する
	/// @param groupName グループ名
	inline void addColliderGroup(const std::string& groupName)
	{
		colliders.emplace(groupName, GroupType());
	}

	/// @brief 指定グループにコライダーを追加する
	/// @param groupName グループ名。指定グループが無ければ新しく作る
	/// @param collider 追加するコライダー
	inline void addCollider(const std::string& groupName,
							GameObj* obj,
							float colliderR)
	{
		auto& i = colliders[groupName].emplace_front(ColliderType{ .obj = obj, .colliderR = colliderR });
	}

	struct GroupAndHitProc
	{
		std::string name;
		std::function<void(GameObj* obj)> hitProc;

		GroupAndHitProc(const std::string& name = "",
						const std::function<void(GameObj* obj)>& hitProc = [](GameObj* obj) {}) :
			name(name), hitProc(hitProc)
		{}
	};

	void checkHitAll(const GroupAndHitProc& group1, const GroupAndHitProc& group2);
};

