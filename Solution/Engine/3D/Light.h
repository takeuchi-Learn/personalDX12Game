#pragma once

#include <wrl.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include "System/DX12Base.h"

class Light
{
private:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	struct ConstBufferData
	{
		DirectX::XMFLOAT3 lightPos{ 0, 0, 0 };		// ライトの位置(ワールド)
		float pad{};
		DirectX::XMFLOAT3 lightColor{ 1, 1, 1 };	// ライト色
	};

private:
	static DX12Base* dxBase;

private:
	ComPtr<ID3D12Resource> constBuff;	// 定数バッファ
	DirectX::XMFLOAT3 pos = { 0, 0, 0 };	// ライトの位置
	DirectX::XMFLOAT3 color = { 1, 1, 1 };	// ライトの色
	bool dirty = false;

public:
	inline const DirectX::XMFLOAT3& getPos() { return pos; }

	// 内部でinit関数を呼び出している
	Light();

	//定数バッファ転送
	void transferConstBuffer();

	void init();

	// 光線の方向をセット
	void setLightPos(const DirectX::XMFLOAT3& lightPos);
	void setLightColor(const DirectX::XMFLOAT3& lightColor);

	void update();

	// @param rootParamIndex : Object3dクラスのcreateGraphicsPipeline関数内のrootParamsの要素数
	void draw(UINT rootParamIndex);
};
