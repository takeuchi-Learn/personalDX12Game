#pragma once

#include <DirectXMath.h>
#include <wrl.h>

#include <d3d12.h>

class SpriteBase
{
private:
	using XMMATRIX = DirectX::XMMATRIX;
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	// ブレンドモード
	enum class BLEND_MODE : USHORT
	{
		ALPHA,	// 半透明合成
		ADD,	// 加算合成
		SUB,	// 減算合成
		REVERSE	// 反転合成
	};

	// パイプラインセット
	struct PipelineSet
	{
		// パイプラインステートオブジェクト
		ComPtr<ID3D12PipelineState> pipelinestate;
		// ルートシグネチャ
		ComPtr<ID3D12RootSignature> rootsignature;
	};

public:
	// テクスチャの最大枚数
	static const UINT spriteSRVCount = 512u;

	// パイプラインセット
	PipelineSet pipelineSet;
	// 射影行列
	XMMATRIX matProjection{};
	// テクスチャ用デスクリプタヒープの生成
	ComPtr<ID3D12DescriptorHeap> descHeap;
	// テクスチャリソース（テクスチャバッファ）の配列
	ComPtr<ID3D12Resource> texBuff[spriteSRVCount];

	// 既に読み込んだ画像(テクスチャ)の数
	static UINT nowTexNum;

private:
	// スプライト用パイプライン生成(createSpriteCommon内で呼び出している)
	static SpriteBase::PipelineSet SpriteCreateGraphicsPipeline(ID3D12Device* dev,
																const wchar_t* vsPath, const wchar_t* psPath,
																BLEND_MODE blendMode);

public:
	SpriteBase(BLEND_MODE blendMode = BLEND_MODE::ALPHA,
			   const wchar_t* vsPath = L"Resources/Shaders/SpriteVS.hlsl",
			   const wchar_t* psPath = L"Resources/Shaders/SpritePS.hlsl");

	// スプライト共通テクスチャ読み込み
	UINT loadTexture(const wchar_t* filename, DirectX::XMFLOAT2* pTexSize = nullptr);

	// スプライト共通グラフィックコマンドのセット
	void drawStart(ID3D12GraphicsCommandList* cmdList);
};
