#pragma once
#include "BaseComposite.h"

/// @brief ビヘイビアツリーのセレクター(成功で終了)
class Selector :
	public BaseComposite
{
	NODE_RESULT mainProc() override;
public:
	using BaseComposite::BaseComposite;
};
