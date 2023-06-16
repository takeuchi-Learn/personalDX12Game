#pragma once

#include "FbxModel.h"

#include <wrl.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

#include "Camera/Camera.h"
#include "3D/Light.h"
#include <3D/BaseObj.h>

class FbxObj3d :
	public BaseObj
{
public:

	// 定数バッファ用データ構造体
	struct ConstBufferDataTransform
	{
		XMFLOAT4 color;
		XMMATRIX viewproj;
		XMMATRIX world;
		XMFLOAT3 cameraPos;
	};

	// ボーンの最大数(hlslの定数と合わせる)
	static const int MAX_BONES = 32;

	struct ConstBufferDataSkin
	{
		XMMATRIX bones[MAX_BONES];
	};
private:
	static DX12Base* dxBase;

	static ComPtr<ID3D12RootSignature> rootsignature;
	static std::vector<ComPtr<ID3D12PipelineState>> pipelinestate;

public:
	static size_t ppStateNum;

	//3Dオブジェクト用パイプライン生成
	// シェーダーモデル指定は "*s_5_0"
	static size_t createGraphicsPipeline(BLEND_MODE blendMode = BLEND_MODE::ALPHA,
										 const wchar_t* vsPath = L"Resources/shaders/FBXVS.hlsl",
										 const wchar_t* psPath = L"Resources/shaders/FBXPS.hlsl");

protected:
	static void startDraw(size_t ppState = ppStateNum);

	// 定数バッファ
	ComPtr<ID3D12Resource> constBuffTransform;

	XMMATRIX modelWorldMat{};

	std::vector<XMFLOAT3> posArr{};

	// 定数バッファ(スキン)
	ComPtr<ID3D12Resource> constBuffSkin;

	FbxTime frameTime;
	FbxTime startTime;
	FbxTime endTime;
	FbxTime currentTime;
	bool isPlay = false;

	bool animLoop = true;

public:

	// モデルデータ
	FbxModel* model = nullptr;

	bool isBillboard = false;
	bool isBillBoardY = false;// isBillboardがfalseの場合のみ機能する

public:

	DirectX::XMFLOAT3 calcVertPos(size_t vertNum);

	// モデル未読み込み
	FbxObj3d(Camera* camera, bool animLoop = true);
	// モデル読み込む
	FbxObj3d(Camera* camera, FbxModel* model, bool animLoop = true);

	void init();	// コンストラクタ内で呼び出している
	void update();
	void draw(Light* light, size_t ppState = ppStateNum);

	void drawWithUpdate(Light* light, size_t ppState = ppStateNum);

	void playAnimation();
	void stopAnimation(bool resetPoseFlag = true);
};
