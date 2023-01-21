#pragma once

#include "FbxModel.h"

#include <wrl.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

#include "Camera/Camera.h"
#include "3D/Light.h"

class Object3d;

class FbxObj3d
{
protected:
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

public:
	static void startDraw();

	//3Dオブジェクト用パイプライン生成
	// シェーダーモデル指定は "*s_5_0"
	static size_t createGraphicsPipeline(BLEND_MODE blendMode = BLEND_MODE::ALPHA,
										 const wchar_t* vsPath = L"Resources/shaders/FBXVS.hlsl",
										 const wchar_t* psPath = L"Resources/shaders/FBXPS.hlsl");


protected:
	Camera* camera = nullptr;

	// 定数バッファ
	ComPtr<ID3D12Resource> constBuffTransform;
	// ワールド変換行列
	XMMATRIX matWorld{};

	XMMATRIX matScale{};
	XMMATRIX matRot{};
	XMMATRIX matTrans{};

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
	// 色
	XMFLOAT4 color = { 1, 1, 1, 1 };

	// アフィン変換情報
	XMFLOAT3 scale = { 1, 1, 1 };
	XMFLOAT3 rotation = { 0, 0, 0 };
	XMFLOAT3 position = { 0, 0, 0 };

	// fbxとobj両方の親がいる場合、fbxが優先される
	FbxObj3d* parent = nullptr;
	// fbxとobj両方の親がいる場合、fbxが優先される
	Object3d* objParent = nullptr;

	// モデルデータ
	FbxModel* model = nullptr;

	bool isBillboard = false;
	bool isBillBoardY = false;// isBillboardがfalseの場合のみ機能する

public:

#pragma region アクセッサ

	inline const XMMATRIX& getMatWorld() const { return matWorld; }

	inline const XMMATRIX& getMatRota() const { return matRot; }
	inline const XMMATRIX& getMatScale() const { return matScale; }
	inline const XMMATRIX& getMatTrans() const { return matTrans; }

	inline void setCamera(Camera* camera) { this->camera = camera; }

#pragma endregion アクセッサ

	inline XMFLOAT3 calcWorldPos() const
	{
		return XMFLOAT3(matWorld.r[3].m128_f32[0],
						matWorld.r[3].m128_f32[1],
						matWorld.r[3].m128_f32[2]);
	}

	DirectX::XMFLOAT2 calcScreenPos();

	DirectX::XMFLOAT3 calcVertPos(size_t vertNum);

	// モデル未読み込み
	FbxObj3d(Camera* camera, bool animLoop = true);
	// モデル読み込む
	FbxObj3d(Camera* camera, FbxModel* model, bool animLoop = true);

	void init();	// コンストラクタ内で呼び出している
	void update();
	void draw(Light* light);

	void drawWithUpdate(Light* light);

	void playAnimation();
	void stopAnimation(bool resetPoseFlag = true);
};
