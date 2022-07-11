#pragma once

#include <DirectXMath.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <forward_list>

#include "Camera.h"
#include "Time.h"

#include "Object3d.h"

class ParticleMgr {
private:
	// エイリアス
   // Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

	// サブクラス
public:
	// 頂点データ構造体
	struct VertexPos {
		XMFLOAT3 pos; // xyz座標
		float scale; // スケール
		XMFLOAT3 color;
	};

	// 定数バッファ用データ構造体
	struct ConstBufferData {
		XMMATRIX mat;	// ビュープロジェクション行列
		XMMATRIX matBillboard;	// ビルボード行列
	};

	// パーティクル1粒
	class Particle {
		// Microsoft::WRL::を省略
		template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
		// DirectX::を省略
		using XMFLOAT2 = DirectX::XMFLOAT2;
		using XMFLOAT3 = DirectX::XMFLOAT3;
		using XMFLOAT4 = DirectX::XMFLOAT4;
		using XMMATRIX = DirectX::XMMATRIX;

	public:
		// 座標
		XMFLOAT3 position = {};
		// 速度
		XMFLOAT3 velocity = {};
		// 加速度
		XMFLOAT3 accel = {};
		// 色
		XMFLOAT3 color = {};
		// スケール
		float scale = 1.0f;
		// 回転
		float rotation = 0.0f;
		// 初期値
		XMFLOAT3 s_color = {};
		float s_scale = 1.0f;
		float s_rotation = 0.0f;
		// 最終値
		XMFLOAT3 e_color = {};
		float e_scale = 0.0f;
		float e_rotation = 0.0f;
		// 現在の時間
		Time::timeType nowTime = 0;
		// 開始時間
		Time::timeType startTime = 0;
		// 終了時間
		Time::timeType life = 0;

		Time* timer = nullptr;
	};

	// 定数
private:
	static const int vertexCount = 0x10000;

	// メンバ変数
private:
	// デバイス
	ID3D12Device* dev = nullptr;
	// デスクリプタサイズ
	UINT descriptorHandleIncrementSize = 0U;
	// ルートシグネチャ
	ComPtr<ID3D12RootSignature> rootsignature;
	// パイプラインステートオブジェクト
	ComPtr<ID3D12PipelineState> pipelinestate;
	// デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeap;
	// 頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	// テクスチャバッファ
	ComPtr<ID3D12Resource> texbuff;
	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView;
	// 定数バッファ
	ComPtr<ID3D12Resource> constBuff;
	// パーティクル配列
	std::forward_list<Particle> particles;
	Camera* camera = nullptr;

	// メンバ関数
public:
	static void ParticleMgr::startDraw(ID3D12GraphicsCommandList* cmdList,
										   Object3d::PipelineSet& ppSet,
										   D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// テクスチャは1x1白で初期化
	ParticleMgr();

	ParticleMgr(const wchar_t* texFilePath, Camera* camera);

	void init(ID3D12Device* device, const wchar_t* texFilePath);
	void update();
	void draw(ID3D12GraphicsCommandList* cmdList);

	void drawWithUpdate(ID3D12GraphicsCommandList* cmdList);

	inline void setCamera(Camera* camera) { this->camera = camera; }

	/// <summary>
	/// パーティクルの追加
	/// </summary>
	/// <param name="life">生存時間</param>
	/// <param name="position">初期座標</param>
	/// <param name="velocity">速度</param>
	/// <param name="accel">加速度</param>
	/// <param name="start_scale">開始時スケール</param>
	/// <param name="end_scale">終了時スケール</param>
	void add(Time* timer, int life,
			 XMFLOAT3 position, XMFLOAT3 velocity, XMFLOAT3 accel,
			 float start_scale, float end_scale,
			 float start_rotation, float end_rotation,
			 XMFLOAT3 start_color, XMFLOAT3 end_color);

	/// <summary>
	/// デスクリプタヒープの初期化
	/// </summary>
	/// <returns></returns>
	void InitializeDescriptorHeap();

	/// <summary>
	/// グラフィックパイプライン生成
	/// </summary>
	/// <returns>成否</returns>
	void InitializeGraphicsPipeline();

	/// <summary>
	/// テクスチャ読み込み
	/// </summary>
	/// <returns>成否</returns>
	void LoadTexture(const wchar_t* filePath);

	/// <summary>
	/// モデル作成
	/// </summary>
	void CreateModel();
};

