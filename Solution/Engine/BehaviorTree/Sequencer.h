#pragma once
#include "BaseComposite.h"

/// @brief ビヘイビアツリーのシーケンサー(失敗で終了)
class Sequencer :
	public BaseComposite
{
public:
	/// @brief 実行
	/// @return 成功したかどうか
	NODE_RESULT run() override;
};
