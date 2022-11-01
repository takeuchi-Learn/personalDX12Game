#pragma once
#include "GameObj.h"
#include <functional>
class BaseEnemy
	: public GameObj
{
protected:
	std::function<void()> phase;

	virtual void afterUpdate() {}

	void additionalUpdate() override;

public:
	BaseEnemy(Camera* camera,
			  ObjModel* model,
			  const DirectX::XMFLOAT3& pos = { 0,0,0 });

	inline void setPhase(const std::function<void()>& _phase) { this->phase = _phase; }
};
