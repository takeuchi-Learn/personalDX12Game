/*****************************************************************//**
 * \file   BaseComposite.h
 * \brief  ビヘイビアツリーのコンポジット基底クラスとノードの結果列挙体
 *********************************************************************/

#pragma once
#include <functional>
#include <vector>

 /// @brief ノードの結果
enum class NODE_RESULT : uint8_t
{
	FAIL,		/// 失敗
	SUCCESS,	/// 成功
	RUNNING,	/// 実行中
};

/// @brief タスククラス
class Task
{
	std::function<NODE_RESULT()> proc;

public:
	Task(const std::function<NODE_RESULT()>& proc) : proc(proc) {}

	inline NODE_RESULT run() { return proc(); }
};

/// @brief コンポジット基底クラス
class BaseComposite :
	public Task
{
protected:
	/// @brief 子ノード
	std::vector<Task> child;

	size_t currentPos = 0u;

	virtual NODE_RESULT mainProc() = 0;

public:
	BaseComposite() : Task(std::bind(&BaseComposite::mainProc, this)) {}

	/// @brief 子ノードを後ろ追加
	/// @param func 子ノードに入れる関数
	inline void addChild(const Task& func)
	{
		child.emplace_back(func);
	}

	inline const auto& getChildList()
	{
		return child;
	}
};
