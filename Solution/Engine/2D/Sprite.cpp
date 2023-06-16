#include "Sprite.h"

#include <string>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <d3dx12.h>

using namespace DirectX;
using namespace Microsoft::WRL;

#include <DirectXTex.h>

// スプライト単体頂点バッファの転送
void Sprite::SpriteTransferVertexBuffer(const SpriteBase* spriteCommon)
{
	HRESULT result = S_FALSE;

	// 頂点データ
	VertexPosUv vertices[] = {
		//     u     v
		{{}, {0.0f, 1.0f}}, // 左下
		{{}, {0.0f, 0.0f}}, // 左上
		{{}, {1.0f, 1.0f}}, // 右下
		{{}, {1.0f, 0.0f}}, // 右上
	};

	// 左下、左上、右下、右上
	enum { LB, LT, RB, RT };

	float left = (0.0f - anchorpoint.x) * size.x;
	float right = (1.0f - anchorpoint.x) * size.x;
	float top = (0.0f - anchorpoint.y) * size.y;
	float bottom = (1.0f - anchorpoint.y) * size.y;

	if (isFlipX)
	{// 左右入れ替え
		left = -left;
		right = -right;
	}

	if (isFlipY)
	{// 上下入れ替え
		top = -top;
		bottom = -bottom;
	}

	vertices[LB].pos = { left, bottom,  0.0f }; // 左下
	vertices[LT].pos = { left, top,     0.0f }; // 左上
	vertices[RB].pos = { right, bottom, 0.0f }; // 右下
	vertices[RT].pos = { right, top,    0.0f }; // 右上

	// 指定番号の画像が読み込み済みなら
	if (spriteCommon->texBuff[texNumber])
	{
		// テクスチャ情報取得
		D3D12_RESOURCE_DESC resDesc = spriteCommon->texBuff[texNumber]->GetDesc();

		float tex_left = texLeftTop.x / resDesc.Width;
		float tex_right = (texLeftTop.x + texSize.x) / resDesc.Width;
		float tex_top = texLeftTop.y / resDesc.Height;
		float tex_bottom = (texLeftTop.y + texSize.y) / resDesc.Height;

		vertices[LB].uv = { tex_left,   tex_bottom }; // 左下
		vertices[LT].uv = { tex_left,   tex_top }; // 左上
		vertices[RB].uv = { tex_right,  tex_bottom }; // 右下
		vertices[RT].uv = { tex_right,  tex_top }; // 右上
	}

	// 頂点バッファへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	vertBuff->Unmap(0, nullptr);
}

Sprite::Sprite(UINT texNumber,
			   const SpriteBase* spriteCommon,
			   XMFLOAT2 anchorpoint,
			   bool isFlipX, bool isFlipY)
{
	create(DX12Base::getInstance()->getDev(),
		   WinAPI::window_width, WinAPI::window_height,
		   texNumber,
		   spriteCommon,
		   anchorpoint,
		   isFlipX, isFlipY);
}

// スプライト生成
void Sprite::create(ID3D12Device* dev, int window_width, int window_height,
					UINT texNumber, const SpriteBase* spriteCommon, XMFLOAT2 anchorpoint,
					bool isFlipX, bool isFlipY)
{
	// テクスチャ番号をコピー
	this->texNumber = texNumber;

	// アンカーポイントをコピー
	this->anchorpoint = anchorpoint;

	// 反転フラグをコピー
	this->isFlipX = isFlipX;
	this->isFlipY = isFlipY;

	// 頂点データ
	VertexPosUv vertices[4]{};

	// 指定番号の画像が読み込み済みなら
	if (spriteCommon->texBuff[texNumber])
	{
		// テクスチャ情報取得
		D3D12_RESOURCE_DESC resDesc = spriteCommon->texBuff[texNumber]->GetDesc();

		// スプライトの大きさを画像の解像度に合わせる
		size = { (float)resDesc.Width, (float)resDesc.Height };
		texSize = { (float)resDesc.Width, (float)resDesc.Height };
	}

	// 頂点バッファ生成
	HRESULT result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertBuff));

	// 頂点バッファデータ転送
	SpriteTransferVertexBuffer(spriteCommon);

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]);

	// 定数バッファの生成
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&constBuff));

	// 定数バッファにデータ転送
	ConstBufferData* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = XMFLOAT4(1, 1, 1, 1); // 色指定（RGBA）
	constMap->mat = XMMatrixOrthographicOffCenterLH(
		0.0f, (float)window_width, (float)window_height, 0.0f, 0.0f, 1.0f);   // 平行投影行列の合成
	constBuff->Unmap(0, nullptr);
}

// スプライト単体更新
void Sprite::update(const SpriteBase* spriteCommon)
{
	if (dirty)
	{
		SpriteTransferVertexBuffer(spriteCommon);
		dirty = false;
	}

	// ワールド行列の更新
	matWorld = XMMatrixIdentity();
	// Z軸回転
	matWorld *= XMMatrixRotationZ(XMConvertToRadians(rotation));
	// 平行移動
	matWorld *= XMMatrixTranslation(position.x, position.y, position.z);

	// 定数バッファの転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->mat = matWorld * spriteCommon->matProjection;
	constMap->color = color;
	constBuff->Unmap(0, nullptr);
}

// スプライト単体描画
void Sprite::draw(ID3D12GraphicsCommandList* cmdList, const SpriteBase* spriteCommon, ID3D12Device* dev)
{
	if (isInvisible)
	{
		return;
	}

	// 頂点バッファをセット
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	// 定数バッファをセット
	cmdList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());

	// シェーダリソースビューをセット
	cmdList->SetGraphicsRootDescriptorTable(1,
											CD3DX12_GPU_DESCRIPTOR_HANDLE(
												spriteCommon->descHeap->GetGPUDescriptorHandleForHeapStart(),
												texNumber,
												dev->GetDescriptorHandleIncrementSize(
													D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
											)
	);

	// ポリゴンの描画（4頂点で四角形）
	cmdList->DrawInstanced(4, 1, 0, 0);
}

// 更新と描画を同時に行う
void Sprite::drawWithUpdate(DX12Base* dxBase,
							const SpriteBase* spriteCommon)
{
	update(spriteCommon);
	draw(dxBase->getCmdList(), spriteCommon, dxBase->getDev());
}