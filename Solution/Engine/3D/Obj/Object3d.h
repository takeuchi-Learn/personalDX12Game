﻿#pragma once

#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <d3d12.h>

#include "ObjModel.h"
#include "System/DX12Base.h"
#include "Camera/Camera.h"
#include "3D/Light.h"

class Object3d
{
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	enum class BLEND_MODE : short
	{
		ALPHA,
		ADD,
		SUB,
		REVERSE
	};

	// 定数バッファ用データ構造体B0
	struct ConstBufferDataB0
	{
		XMFLOAT4 color;
		XMMATRIX viewProj;
		XMMATRIX world;	// ワールド行列
		XMFLOAT3 cameraPos;	// カメラ位置(ワールド座標)
	};

	// --------------------
	// staticメンバ
	// --------------------
private:
	static DX12Base* dxBase;
	static size_t ppStateDefNum;
	static ComPtr<ID3D12RootSignature> rootsignature;
	static std::vector<ComPtr<ID3D12PipelineState>> pipelinestate;
	static size_t ppStateNum;

	static void createTransferBufferB0(ComPtr<ID3D12Resource>& constBuff);

public:
	static void startDraw(size_t ppStateNum = ppStateDefNum,
						  D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//3Dオブジェクト用パイプライン生成
	// シェーダーモデル指定は "*s_5_0"
	static size_t createGraphicsPipeline(BLEND_MODE blendMode = BLEND_MODE::ALPHA,
										 const wchar_t* vsPath = L"Resources/Shaders/BasicVS.hlsl",
										 const wchar_t* psPath = L"Resources/Shaders/BasicPS.hlsl");

	static inline size_t getGraphicsPipeline() { return ppStateDefNum; }

	static void staticInit();

	// --------------------
	// (動的)メンバ
	// --------------------
private:
	Camera* camera = nullptr;

	// 定数バッファ
	ComPtr<ID3D12Resource> constBuffB0;
	// ワールド変換行列
	XMMATRIX matWorld{};

	XMMATRIX matScale{};
	XMMATRIX matRot{};
	XMMATRIX matTrans{};

public:
	// 色
	XMFLOAT4 color = { 1, 1, 1, 1 };

	// アフィン変換情報
	XMFLOAT3 scale = { 1, 1, 1 };
	XMFLOAT3 rotation = { 0, 0, 0 };
	XMFLOAT3 position = { 0, 0, 0 };

	// 親オブジェクトへのポインタ
	Object3d* parent = nullptr;

	//モデルデータ
	ObjModel* model = nullptr;

	bool isBillboard = false;
	bool isBillBoardY = false;// isBillboardがfalseの場合のみ機能する

#pragma region アクセッサ

	inline const XMMATRIX& getMatWorld() const { return matWorld; }

	inline const XMMATRIX& getMatRota() const { return matRot; }
	inline const XMMATRIX& getMatScale() const { return matScale; }
	inline const XMMATRIX& getMatTrans() const { return matTrans; }

	inline const Camera* getCamera() const { return camera; }

#pragma endregion アクセッサ

	inline XMFLOAT3 calcWorldPos() const
	{
		return XMFLOAT3(matWorld.r[3].m128_f32[0],
						matWorld.r[3].m128_f32[1],
						matWorld.r[3].m128_f32[2]);
	}

	XMFLOAT2 calcScreenPos();

	// モデルは後から手動で読み込む(deleteも手動)
	Object3d(Camera* camera);

	// モデルデータもここで渡す(deleteは手動)
	Object3d(Camera* camera, ObjModel* model);

	~Object3d();

	void update();

	void draw(DX12Base* dxCom, Light* light);

	void drawWithUpdate(DX12Base* dxCom, Light* light);
};
