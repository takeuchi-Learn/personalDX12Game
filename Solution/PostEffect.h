﻿#pragma once
#include "DX12Base.h"

#include "Time.h"

#include <memory>
#include <vector>

class PostEffect {
	PostEffect();
	~PostEffect() = default;
	PostEffect(const PostEffect &obj) = delete;
	void operator=(const PostEffect &obj) = delete;

public:
	// レンダーターゲットの数 = このクラスのテクスチャバッファの数
	// シェーダーに合わせる
	static const UINT renderTargetNum = 2;

	static PostEffect *getInstance();

private:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 頂点データ
	struct VertexPosUv {
		DirectX::XMFLOAT3 pos; // xyz座標
		DirectX::XMFLOAT2 uv;  // uv座標
	};

	// 定数バッファ用データ構造体
	struct ConstBufferData {
		float oneSec;
		float nowTime;
		DirectX::XMFLOAT2 winSize;
		float noiseIntensity;
		DirectX::XMFLOAT2 mosaicNum;
		float vignIntensity;
		float alpha;	// 不透明度(通常は1)
	};

	// パイプラインセット
	struct PipelineSet {
		// パイプラインステートオブジェクト
		ComPtr<ID3D12PipelineState> pipelinestate;
		// ルートシグネチャ
		ComPtr<ID3D12RootSignature> rootsignature;
	};


	//頂点バッファ;
	ComPtr<ID3D12Resource> vertBuff;
	//頂点バッファビュー;
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//定数バッファ;
	ComPtr<ID3D12Resource> constBuff;

	// テクスチャバッファ
	ComPtr<ID3D12Resource> texbuff[renderTargetNum];
	// SRV用のデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeapSRV;

	// 深度バッファ
	ComPtr<ID3D12Resource> depthBuff;
	// RTV用デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeapRTV;
	// DSV用デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeapDSV;

	// 画面クリアの色
	static const float clearColor[4];

	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12RootSignature> rootSignature;

	// パイプラインとルートシグネチャのセット
	std::vector<PipelineSet> pipelineSet;
	UINT nowPPSet = 0u;

	std::unique_ptr<Time> timer;

	float noiseIntensity;
	DirectX::XMFLOAT2 mosaicNum;
	float vignIntensity;
	float alpha;

	ID3D12Device *dev;
	ID3D12GraphicsCommandList *cmdList;

private:
	static const wchar_t *vsPathDef;

private:
	void initBuffer();

	void createGraphicsPipelineState(const wchar_t *vsPath = L"Resources/Shaders/PostEffectVS.hlsl",
									 const wchar_t *psPath = L"Resources/Shaders/PostEffectPS.hlsl");

	void transferConstBuff(float nowTime, float oneSec = Time::oneSec);

	void init();

public:
	// @param 0 ~ 1
	inline void setNoiseIntensity(float intensity) { noiseIntensity = intensity; }
	inline float getNoiseIntensity() const { return noiseIntensity; }

	inline void setMosaicNum(const DirectX::XMFLOAT2 &mosaicNum) { this->mosaicNum = mosaicNum; }
	inline DirectX::XMFLOAT2 getMosaicNum() const { return mosaicNum; }

	inline void setAlpha(float alpha) { this->alpha = alpha; }
	inline float getAlpha() const { return alpha; }

	/// <summary>
	/// グラフィックスパイプラインを追加
	/// </summary>
	/// <param name="psPath">ピクセルシェーダーファイルのパス</param>
	/// <returns>識別番号</returns>
	size_t addPipeLine(const wchar_t *psPath);

	/// <summary>
	/// グラフィックスパイプラインの切り替え
	/// </summary>
	/// <param name="GPPNum">識別番号</param>
	inline void changePipeLine(UINT GPPNum) { nowPPSet = GPPNum; }

	/// <summary>
	/// 現在のグラフィックスパイプラインの識別番号を取得
	/// </summary>
	/// <returns>識別番号</returns>
	inline const UINT getPipeLineNum() { return nowPPSet; }


	void draw(DX12Base *dxCom);

	void startDrawScene(DX12Base *dxCom);

	void endDrawScene(DX12Base *dxCom);
};

