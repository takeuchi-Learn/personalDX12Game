#include "Light.h"
#include <cassert>
#include <d3dx12.h>

using namespace DirectX;

DX12Base* Light::dxBase = DX12Base::getInstance();

Light::Light()
{
	init();
}

void Light::transferConstBuffer()
{
	HRESULT result = S_FALSE;
	//定数バッファへデータ転送
	ConstBufferData* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result))
	{
		constMap->lightPos = pos;
		constMap->lightColor = color;
		constBuff->Unmap(0, nullptr);
	}
}

void Light::init()
{
	//定数バッファ生成
	HRESULT result = dxBase->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff)
	);
	assert(SUCCEEDED(result));

	// 定数バッファへデータ転送
	transferConstBuffer();
}

void Light::setLightPos(const DirectX::XMFLOAT3& lightPos)
{
	pos = lightPos;
	dirty = true;
}

void Light::setLightColor(const DirectX::XMFLOAT3& lightColor)
{
	this->color = lightColor;
	dirty = true;
}

void Light::update()
{
	if (dirty)
	{
		transferConstBuffer();
		dirty = false;
	}
}

void Light::draw(UINT rootParamIndex)
{
	dxBase->getCmdList()->SetGraphicsRootConstantBufferView(rootParamIndex,
															constBuff->GetGPUVirtualAddress());
}