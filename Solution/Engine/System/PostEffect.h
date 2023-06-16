#pragma once
#include "System/DX12Base.h"

#include <memory>
#include <vector>
#include "Util/Timer.h"

class PostEffect
{
	PostEffect();
	~PostEffect() = default;
	PostEffect(const PostEffect& obj) = delete;
	void operator=(const PostEffect& obj) = delete;

public:
	// レンダーターゲットの数 = このクラスのテクスチャバッファの数
	// シェーダーに合わせる
	static const UINT renderTargetNum = 2;

	inline static PostEffect* getInstance()
	{
		static PostEffect ps{};
		return &ps;
	}

private:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 頂点データ
	struct VertexPosUv
	{
		DirectX::XMFLOAT3 pos; // xyz座標
		DirectX::XMFLOAT2 uv;  // uv座標
	};

	// 定数バッファ用データ構造体
	struct ConstBufferData
	{
		float oneSec;
		float nowTime;
		DirectX::XMFLOAT2 winSize;
		float noiseIntensity;
		DirectX::XMFLOAT2 mosaicNum;
		float vignIntensity;
		float alpha;	// 不透明度(通常は1)
		DirectX::XMFLOAT2 rgbShiftNum;
		float speedLineIntensity;
	};

	// パイプラインセット
	struct PipelineSet
	{
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

	std::unique_ptr<Timer> timer;

	float noiseIntensity;
	DirectX::XMFLOAT2 mosaicNum;
	float vignIntensity;
	float alpha;
	DirectX::XMFLOAT2 rgbShiftNum;
	float speedLineIntensity;

	ID3D12Device* dev;
	ID3D12GraphicsCommandList* cmdList;

private:
	void initBuffer();

	void createGraphicsPipelineState(const wchar_t* psPath = L"Resources/Shaders/PostEffectPS.hlsl");

	void transferConstBuff(float nowTime, float oneSec = Timer::oneSec);

	void init();

public:
	// @param 0 ~ 1
	inline void setNoiseIntensity(float intensity) { noiseIntensity = intensity; }
	inline float getNoiseIntensity() const { return noiseIntensity; }

	inline void setMosaicNum(const DirectX::XMFLOAT2& mosaicNum) { this->mosaicNum = mosaicNum; }
	inline DirectX::XMFLOAT2 getMosaicNum() const { return mosaicNum; }

	inline void setRgbShiftNum(const DirectX::XMFLOAT2& shiftNum) { this->rgbShiftNum = shiftNum; }
	inline const DirectX::XMFLOAT2& getRgbShiftNum() const { return rgbShiftNum; }

	inline void setAlpha(float alpha) { this->alpha = alpha; }
	inline float getAlpha() const { return alpha; }

	inline void setVignIntensity(float num) { vignIntensity = num; }
	inline float getVignIntensity() const { return vignIntensity; }

	inline void setSpeedLineIntensity(float num) { this->speedLineIntensity = num; }
	inline float getSpeedLineIntensity() const { return speedLineIntensity; }

	/// @brief グラフィックスパイプラインを追加
	/// @param psPath ピクセルシェーダーファイルのパス
	/// @return 識別番号
	size_t addPipeLine(const wchar_t* psPath);

	/// @brief グラフィックスパイプラインの切り替え
	/// @param GPPNum 識別番号
	inline void changePipeLine(UINT GPPNum) { nowPPSet = GPPNum; }

	/// @brief 現在のグラフィックスパイプラインの識別番号を取得
	/// @return 識別番号
	inline const UINT getPipeLineNum() { return nowPPSet; }

	void draw(DX12Base* dxCom);

	void startDrawScene(DX12Base* dxCom);

	void endDrawScene(DX12Base* dxCom);
};
