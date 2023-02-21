﻿#include "Selector.h"

NODE_RESULT Selector::mainProc()
{
	for (size_t i = currentPos;
		 i < child.size();
		 ++i)
	{
		// ノードの実行結果
		const auto res = child[i].run();

		// 実行中なら現在位置を記録して終了
		if (res == NODE_RESULT::RUNNING)
		{
			currentPos = i;
			return NODE_RESULT::RUNNING;
		}

		// 実行が終了していたら記録していた現在位置をリセット
		currentPos = 0u;

		// セレクターなので成功で終了
		if (res == NODE_RESULT::SUCCESS)
		{
			return NODE_RESULT::SUCCESS;
		}
	}
	// すべて失敗なら失敗を返す
	return NODE_RESULT::FAIL;
}
