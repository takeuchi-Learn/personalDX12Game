/*****************************************************************//**
 * \file   BaseComposite.h
 * \brief  ビヘイビアツリーのコンポジット基底クラスとノードの結果列挙体
 *********************************************************************/

#pragma once
#include <functional>
#include <list>

 /// @brief ノードの結果
enum class NODE_RESULT : uint8_t
{
	FAIL,		/// 失敗
	SUCCESS,	/// 成功
};

/// @brief タスククラス
using Task = std::function<NODE_RESULT()>;

/// @brief コンポジット基底クラス
class BaseComposite
{
public:
	/// @brief 実行
	/// @return 成功したかどうか
	virtual NODE_RESULT run() = 0;

	/// @brief 子ノードを後ろ追加
	/// @param func 子ノードに入れる関数
	inline void addChild(Task func)
	{
		child.emplace_back(func);
	}

	inline const auto& getChildList()
	{
		return child;
	}

protected:
	/// @brief 子ノード
	std::list<Task> child;
};
