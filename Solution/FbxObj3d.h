#pragma once

#include "FbxModel.h"
#include "Camera.h"

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <string>

#include "Light.h"

class FbxObj3d {
protected:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	// ボーンの最大数(hlslの定数と合わせる)
	static const int MAX_BONES = 32;

	struct ConstBufferDataTransform {
		XMMATRIX viewproj;
		XMMATRIX world;
		XMFLOAT3 cameraPos;
	};

	struct ConstBufferDataSkin {
		XMMATRIX bones[MAX_BONES];
	};

public:
	static void setDevice(ID3D12Device* dev) { FbxObj3d::dev = dev; }
	static void setCamera(Camera* camera) { FbxObj3d::camera = camera; }

	static void createGraphicsPipeline(const wchar_t* vsPath = L"Resources/shaders/FBXVS.hlsl",
									   const wchar_t* psPath = L"Resources/shaders/FBXPS.hlsl");

private:
	static ID3D12Device* dev;
	static Camera* camera;

	static ComPtr<ID3D12RootSignature> rootsignature;
	static ComPtr<ID3D12PipelineState> pipelinestate;

public:
	// モデル未読み込み
	FbxObj3d(bool animLoop = true);
	// モデル読み込む
	FbxObj3d(FbxModel* model, bool animLoop = true);


	void init();	// コンストラクタ内で呼び出している
	void update();
	void draw(ID3D12GraphicsCommandList* cmdList, Light* light);

	void drawWithUpdate(ID3D12GraphicsCommandList* cmdList, Light* light);

	inline void setModel(FbxModel* model) { this->model = model; }

	inline void setScale(const XMFLOAT3& scale) { this->scale = scale; }

	inline void setPosition(const XMFLOAT3& position) { this->position = position; }

	inline void setRotation(const XMFLOAT3& rotation) { this->rotation = rotation; }

	void playAnimation();
	void stopAnimation(bool resetPoseFlag = true);

protected:
	ComPtr<ID3D12Resource> constBuffTransform;

	XMFLOAT3 scale = { 1, 1, 1 };
	XMFLOAT3 rotation = { 0, 0, 0 };
	XMFLOAT3 position = { 0, 0, 0 };
	XMMATRIX matWorld;
	FbxModel* model = nullptr;

	// 定数バッファ(スキン)
	ComPtr<ID3D12Resource> constBuffSkin;

	FbxTime frameTime;
	FbxTime startTime;
	FbxTime endTime;
	FbxTime currentTime;
	bool isPlay = false;

	bool animLoop = true;
};

