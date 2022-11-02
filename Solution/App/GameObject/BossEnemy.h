#pragma once
#include "BaseEnemy.h"
class BossEnemy :
	public BaseEnemy
{
private:
	uint16_t hp;

public:
	BossEnemy(Camera* camera,
			  ObjModel* model,
			  const DirectX::XMFLOAT3& pos = { 0,0,0 },
			  uint16_t hp = 3u);

	/// @brief ダメージを与える
	/// @param damegeNum 与えるダメージ数
	/// @param killFlag hpが0になったらkillするかどうか(trueでkillする)
	/// @return 倒したかどうか(倒したらtrue)
	bool damage(uint16_t damegeNum, bool killFlag = true);
};

