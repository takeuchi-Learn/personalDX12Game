#pragma once

#include <DirectXMath.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <forward_list>
#include "Camera/Camera.h"
#include "Util/Timer.h"

class ParticleMgr
{
public:
	enum ParticleMgr_BLENDMODE : uint8_t
	{
		ADD = 0u,
		SUB = 1u
	};

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
	struct VertexPos
	{
		XMFLOAT3 pos; // xyz座標
		float scale; // スケール
		XMFLOAT3 color;
	};

	// 定数バッファ用データ構造体
	struct ConstBufferData
	{
		XMMATRIX mat;	// ビュープロジェクション行列
		XMMATRIX matBillboard;	// ビルボード行列
	};

	// パーティクル1粒
	class Particle
	{
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
		XMFLOAT3 s_position = {};

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
		Timer::timeType nowTime = 0;
		// 開始時間
		Timer::timeType startTime = 0;
		// 終了時間
		Timer::timeType life = 0;

		std::unique_ptr<Timer> timer = nullptr;
	};

	// 定数
private:
	static DX12Base* dxBase;

	static const int vertexCount = 0x10000;

public:
	static void createParticle(ParticleMgr* particleMgr,
							   const XMFLOAT3& pos,
							   const uint16_t particleNum = 10U,
							   const float startScale = 1.f,
							   const float vel = 5.f,
							   const XMFLOAT3& startCol = { 1.f, 1.f, 0.25f },
							   const XMFLOAT3& endCol = { 1.f, 0.f, 1.f });

	// メンバ変数
private:
	// デスクリプタサイズ
	UINT descriptorHandleIncrementSize = 0U;
	// ルートシグネチャ
	ComPtr<ID3D12RootSignature> rootsignature;
	// パイプラインステートオブジェクト
	ComPtr<ID3D12PipelineState> pipelinestate[2];
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
	std::forward_list<std::unique_ptr<Particle>> particles;
	Camera* camera = nullptr;

public:
	/// @brief 現在のブレンドモード
	ParticleMgr_BLENDMODE nowBlendMode = ParticleMgr_BLENDMODE::ADD;

	// メンバ関数
public:
	/// @brief ブレンドモードを切り替え
	/// @return 切り替え後のブレンドモード
	inline ParticleMgr_BLENDMODE changeBlendMode() const
	{
		nowBlendMode == ADD ? SUB : ADD;
		return nowBlendMode;
	}

	// テクスチャは1x1白で初期化
	ParticleMgr();

	ParticleMgr(const wchar_t* texFilePath, Camera* camera);

	void init(const wchar_t* texFilePath);
	void update();
	void draw();

	void drawWithUpdate();

	inline void setCamera(Camera* camera) { this->camera = camera; }

	/// @brief パーティクルの追加
	/// @param life 生存時間
	/// @param position 初期座標
	/// @param velocity 初速度
	/// @param accel 加速度
	/// @param start_scale 開始時スケール
	/// @param end_scale 終了時スケール
	/// @param start_rotation 開始時の回転
	/// @param end_rotation 終了時の回転
	/// @param start_color 開始時の色
	/// @param end_color 終了時の色
	void add(Timer::timeType life,
			 const XMFLOAT3& position, const XMFLOAT3& velocity, const XMFLOAT3& accel,
			 float start_scale, float end_scale,
			 float start_rotation, float end_rotation,
			 const XMFLOAT3& start_color, const XMFLOAT3& end_color);

	/// @brief デスクリプタヒープの初期化
	void InitializeDescriptorHeap();

	/// @brief グラフィックパイプライン生成
	void InitializeGraphicsPipeline();

	/// @brief テクスチャ読み込み
	/// @param filePath テクスチャファイルのパス
	void LoadTexture(const wchar_t* filePath);

	/// @brief モデル作成
	void CreateModel();
};
