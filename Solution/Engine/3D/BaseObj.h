#pragma once
#include <DirectXMath.h>
#include <Camera/Camera.h>
#include <3D/Light.h>

class BaseObj
{
protected:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	enum class BLEND_MODE : short
	{
		ALPHA,
		ADD,
		SUB,
		REVERSE
	};

protected:

	Camera* camera = nullptr;

	// ワールド変換行列
	XMMATRIX matWorld{};

	XMMATRIX matScale{};
	XMMATRIX matRot{};
	XMMATRIX matTrans{};

public:

	// 色
	XMFLOAT4 color = { 1, 1, 1, 1 };

	// アフィン変換情報
	XMFLOAT3 scale = { 1, 1, 1 };
	XMFLOAT3 rotation = { 0, 0, 0 };
	XMFLOAT3 position = { 0, 0, 0 };

	BaseObj* parent = nullptr;

	bool isBillboard = false;
	bool isBillBoardY = false;// isBillboardがfalseの場合のみ機能する

	bool drawFlag = true;

protected:
	void updateMatWorld();

public:

	BaseObj(Camera* camera);

	inline XMFLOAT3 calcWorldPos() const
	{
		return XMFLOAT3(matWorld.r[3].m128_f32[0],
						matWorld.r[3].m128_f32[1],
						matWorld.r[3].m128_f32[2]);
	}

	XMFLOAT2 calcScreenPos();
	inline XMFLOAT3 calcScreenPosF3(float z = 0.f)
	{
		const XMFLOAT2 pos = calcScreenPos();
		return XMFLOAT3(pos.x, pos.y, z);
	}

#pragma region アクセッサ

	inline const XMMATRIX& getMatWorld() const { return matWorld; }

	inline const XMMATRIX& getMatRota() const { return matRot; }
	inline const XMMATRIX& getMatScale() const { return matScale; }
	inline const XMMATRIX& getMatTrans() const { return matTrans; }

	inline const Camera* getCamera() const { return camera; }
	inline void setCamera(Camera* camera) { this->camera = camera; }

#pragma endregion アクセッサ

	virtual void update() = 0;
	virtual void draw(Light* light, size_t ppState) = 0;
};
