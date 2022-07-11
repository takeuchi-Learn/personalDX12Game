#include "Material.h"
#include <DirectXTex.h>
#include <cassert>
#include "SpriteCommon.h"

using namespace DirectX;

ID3D12Device* Material::dev = nullptr;

void Material::staticInit(ID3D12Device* dev) {
	assert(!Material::dev);
	Material::dev = dev;
}

Material::Material()
	: ambient({ 0.3f,0.3f,0.3f }),
	diffuse({ 0.f,0.f,0.f }),
	specular({ 0.f,0.f,0.f }),
	alpha(1.f) {
	createConstBuff();
	texbuff.resize(Material::maxTexNum);
}

void Material::loadTexture(const std::string& directoryPath, UINT texNum,
						   CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
						   CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle) {

	cpuDescHandleSRV = cpuHandle;
	gpuDescHandleSRV = gpuHandle;

	HRESULT result = S_FALSE;

	// WICテクスチャのロード
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	std::string filepath = directoryPath + texFileName;

	constexpr size_t wfilePathSize = 128;
	wchar_t wfilepath[wfilePathSize];
	MultiByteToWideChar(CP_ACP, 0, filepath.c_str(), -1, wfilepath, wfilePathSize);

	result = LoadFromWICFile(
		wfilepath, WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if (FAILED(result)) {
		assert(0);
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // 生データ抽出

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// テクスチャ用バッファの生成
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texbuff[texNum]));
	if (FAILED(result)) {
		assert(0);
	}

	// テクスチャバッファにデータ転送
	result = texbuff[texNum]->WriteToSubresource(
		0,
		nullptr, // 全領域へコピー
		img->pixels,    // 元データアドレス
		(UINT)img->rowPitch,  // 1ラインサイズ
		(UINT)img->slicePitch // 1枚サイズ
	);
	if (FAILED(result)) {
		assert(0);
	}

	// シェーダリソースビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texbuff[texNum]->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	dev->CreateShaderResourceView(texbuff[texNum].Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		cpuDescHandleSRV
	);
}

void Material::update() {
	HRESULT result = S_FALSE;
	// 定数バッファへデータ転送
	ConstBufferDataB1* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result)) {
		constMap->ambient = ambient;
		constMap->diffuse = diffuse;
		constMap->specular = specular;
		constMap->alpha = alpha;
		constBuff->Unmap(0, nullptr);
	}
}

void Material::createConstBuff() {
	HRESULT result = S_FALSE;
	// 定数バッファの生成
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB1) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	if (FAILED(result)) {
		assert(0);
	}
}
