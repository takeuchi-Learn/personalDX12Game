#include "Sequencer.h"

NODE_RESULT Sequencer::mainProc()
{
	for (size_t i = currentPos;
		 i < child.size();
		 ++i)
	{
		// ノードの実行結果
		const NODE_RESULT res = child[i].run();

		// 実行中なら現在位置を記録して終了
		if (res == NODE_RESULT::RUNNING)
		{
			currentPos = i;
			return NODE_RESULT::RUNNING;
		}

		// 実行が終了していたら記録していた現在位置をリセット
		currentPos = 0u;

		// シーケンサーなので失敗で終了
		if (res == NODE_RESULT::FAIL)
		{
			return NODE_RESULT::FAIL;
		}
	}
	// すべて成功なら成功を返す
	return NODE_RESULT::SUCCESS;
}