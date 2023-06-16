#pragma once

#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <d3d12.h>

#include "ObjModel.h"
#include "System/DX12Base.h"
#include "Camera/Camera.h"
#include "3D/Light.h"

#include <3D/BaseObj.h>

class Object3d :
	public BaseObj
{
public:

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
	static ComPtr<ID3D12RootSignature> rootsignature;
	static std::vector<ComPtr<ID3D12PipelineState>> pipelinestate;

	static void createTransferBufferB0(ComPtr<ID3D12Resource>& constBuff);

	static void startDraw(size_t ppStateNum = ppStateNum);

public:
	static size_t ppStateNum;

	//3Dオブジェクト用パイプライン生成
	// シェーダーモデル指定は "*s_5_0"
	static size_t createGraphicsPipeline(BaseObj::BLEND_MODE blendMode = BaseObj::BLEND_MODE::ALPHA,
										 const wchar_t* vsPath = L"Resources/Shaders/BasicVS.hlsl",
										 const wchar_t* psPath = L"Resources/Shaders/BasicPS.hlsl");

	static inline size_t getGraphicsPipeline() { return ppStateNum; }

	static void staticInit();

	// --------------------
	// (動的)メンバ
	// --------------------
private:
	// 定数バッファ
	ComPtr<ID3D12Resource> constBuffB0;

public:

	//モデルデータ
	ObjModel* model = nullptr;

	// モデルは後から手動で読み込む(deleteも手動)
	Object3d(Camera* camera);

	// モデルデータもここで渡す(deleteは手動)
	Object3d(Camera* camera, ObjModel* model);

	~Object3d();

	void update();

	void draw(Light* light, size_t ppState = ppStateNum);

	void drawWithUpdate(Light* light, size_t ppState = ppStateNum);
};
