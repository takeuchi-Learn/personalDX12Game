#pragma once
#include <DirectXMath.h>

class Player {
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMVECTOR = DirectX::XMVECTOR;

	XMVECTOR lookVec{};

	XMVECTOR pos{};

public:
	Player()
		: lookVec(DirectX::XMVectorSet(0, 0, 1, 0)),
		pos(DirectX::XMVectorSet(0, 0, 0, 1)) {
	}
	inline const XMVECTOR &getLookVec() { return lookVec; }
	inline void setLookVec(const XMVECTOR &lookVec) { this->lookVec = lookVec; }

	inline XMVECTOR getPosVec() { return pos; }
	inline XMFLOAT3 getPosF3() {
		XMFLOAT3 ret{};
		DirectX::XMStoreFloat3(&ret, pos);
		return ret;
	}
	inline void setPos(const XMVECTOR &newPos) { pos = newPos; }
	inline void setPos(const XMFLOAT3 &newPos) { pos = DirectX::XMLoadFloat3(&newPos); }

	void moveForward(float moveVel);

	void moveRight(float moveVel, bool moveYFlag = false);
};

