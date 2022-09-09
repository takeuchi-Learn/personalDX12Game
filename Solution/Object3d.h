﻿#pragma once

#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <d3d12.h>

#include "ObjModel.h"
#include "DX12Base.h"
#include "Camera.h"

#include "Light.h"

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

	//パイプラインセット
	struct PipelineSet
	{
		//パイプラインステートオブジェクト
		ComPtr<ID3D12PipelineState> pipelinestate;
		//ルートシグネチャ
		ComPtr<ID3D12RootSignature> rootsignature;
	};

	// 頂点データ構造体
	//struct Vertex {
	//	XMFLOAT3 pos; // xyz座標
	//	XMFLOAT3 normal; // 法線ベクトル
	//	XMFLOAT2 uv; // uv座標
	//};

	// 定数バッファ用データ構造体B0
	struct ConstBufferDataB0
	{
		XMMATRIX viewProj;
		XMMATRIX world;	// ワールド行列
		XMFLOAT3 cameraPos;	// カメラ位置(ワールド座標)
	};

	// --------------------
	// staticメンバ
	// --------------------
private:
	static DX12Base* dxBase;
	static PipelineSet ppSetDef;
	Camera* camera;

	static void createTransferBufferB0(ComPtr<ID3D12Resource>& constBuff);

	inline XMFLOAT3 subFloat3(const XMFLOAT3& left, const XMFLOAT3& right)
	{
		return XMFLOAT3(left.x - right.x,
						left.y - right.y,
						left.z - right.z);
	}

public:
	// 頂点バッファの最大数
	static const int constantBufferNum = 128;

	static inline PipelineSet& getGraphicsPipeline() { return ppSetDef; }

	static void startDraw(Object3d::PipelineSet& ppSet = getGraphicsPipeline(),
						  D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	static void staticInit();

	//3Dオブジェクト用パイプライン生成
	// シェーダーモデル指定は "*s_5_0"
	static Object3d::PipelineSet createGraphicsPipeline(BLEND_MODE blendMode = BLEND_MODE::ALPHA,
														const wchar_t* vsShaderPath = L"Resources/Shaders/BasicVS.hlsl",
														const wchar_t* psShaderPath = L"Resources/Shaders/BasicPS.hlsl");

	// --------------------
	// (動的)メンバ
	// --------------------
private:
	// 定数バッファ
	ComPtr<ID3D12Resource> constBuffB0;
	// ワールド変換行列
	XMMATRIX matWorld;

	XMMATRIX matScale{};
	XMMATRIX matRot{};
	XMMATRIX matTrans{};

public:
	UINT texNum = 0;

	XMFLOAT4 color = { 1, 1, 1, 1 };

	// アフィン変換情報
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	// 親オブジェクトへのポインタ
	Object3d* parent = nullptr;

	//モデルデータ
	ObjModel* model = nullptr;

	bool isBillboard = false;
	bool isBillBoardY = false;// isBillboardがfalseの場合のみ機能する

	inline const XMMATRIX& getMatWorld() const { return matWorld; }

	//void setTexture(ID3D12Device* dev, const UINT newTexNum);

	inline const XMMATRIX& getMatRota() const { return matRot; }
	inline const XMMATRIX& getMatScale() const { return matScale; }
	inline const XMMATRIX& getMatTrans() const { return matTrans; }

	inline const Camera* getCamera() const { return camera; }

	XMFLOAT2 calcScreenPos();

	// モデルは後から手動で読み込む(deleteも手動)
	Object3d(Camera* camera);

	// モデルデータもここで渡す(deleteは手動)
	Object3d(Camera* camera, ObjModel* model, const UINT texNum);

	void update();

	void draw(DX12Base* dxCom, Light* light);

	void drawWithUpdate(DX12Base* dxCom, Light* light);

	~Object3d();
};
