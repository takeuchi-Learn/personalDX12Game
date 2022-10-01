#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <string>

class Material
{
public:
	// 定数バッファ用データ構造体B1
	struct ConstBufferDataB1
	{
		DirectX::XMFLOAT3 ambient;	// アンビエント
		float pad1;		// パディング
		DirectX::XMFLOAT3 diffuse;	// ディフューズ
		float pad2;		// パディング
		DirectX::XMFLOAT3 specular;	// スペキュラー
		float alpha;	// アルファ
		DirectX::XMFLOAT2 texTilling;	// タイリング
		DirectX::XMFLOAT2 pad3;	// パディング
		DirectX::XMFLOAT2 shiftUv;	// タイリング
	};

public:
	static const uint16_t maxTexNum = 128;
	static void staticInit(ID3D12Device* dev);

private:
	static ID3D12Device* dev;

	// テクスチャバッファ
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> texbuff;
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff;
	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;

public:
	std::string name;			// マテリアル名
	DirectX::XMFLOAT3 ambient;	// アンビエント影響度
	DirectX::XMFLOAT3 diffuse;	// ディフューズ影響度
	DirectX::XMFLOAT3 specular;	// スぺキュラー影響度
	float alpha;
	std::string texFileName;	// テクスチャファイル名
	DirectX::XMFLOAT2 texTilling = { 1, 1 };	// タイリング
	DirectX::XMFLOAT2 shiftUv = { 0, 0 };	// UVシフト

	Material();

	inline ID3D12Resource* getConstBuff() { return constBuff.Get(); }

	void loadTexture(const std::string& directoryPath, UINT texNum, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	void update();

	inline const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetCpuHandle() const { return cpuDescHandleSRV; }
	inline const CD3DX12_GPU_DESCRIPTOR_HANDLE& GetGpuHandle() const { return gpuDescHandleSRV; }

private:
	void createConstBuff();
};
