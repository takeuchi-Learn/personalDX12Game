﻿#pragma once
#include "BaseComposite.h"

/// @brief ビヘイビアツリーのシーケンサー(失敗で終了)
class Sequencer :
	public BaseComposite
{
	NODE_RESULT mainProc();
public:
	using BaseComposite::BaseComposite;
};
