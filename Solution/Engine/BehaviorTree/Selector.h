#pragma once
#include "BaseComposite.h"

/// @brief ビヘイビアツリーのセレクター(成功で終了)
class Selector :
	public BaseComposite
{
public:
	/// @brief 実行
	/// @return 成功したかどうか
	NODE_RESULT run() override;
};
