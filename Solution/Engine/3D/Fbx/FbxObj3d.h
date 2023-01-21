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

	// ボーンの最大数(hlslの定数と合わせる)
	static const int MAX_BONES = 32;

	struct ConstBufferDataTransform
	{
		XMFLOAT4 color;
		XMMATRIX viewproj;
		XMMATRIX world;
		XMFLOAT3 cameraPos;
	};

	struct ConstBufferDataSkin
	{
		XMMATRIX bones[MAX_BONES];
	};

public:
	static size_t createGraphicsPipeline(BLEND_MODE blendMode = BLEND_MODE::ALPHA,
										 const wchar_t* vsPath = L"Resources/shaders/FBXVS.hlsl",
										 const wchar_t* psPath = L"Resources/shaders/FBXPS.hlsl");

private:
	static DX12Base* dxBase;

	static ComPtr<ID3D12RootSignature> rootsignature;
	static std::vector<ComPtr<ID3D12PipelineState>> pipelinestate;

public:
	static size_t ppStateNum;

	// モデル未読み込み
	FbxObj3d(bool animLoop = true);
	// モデル読み込む
	FbxObj3d(FbxModel* model, bool animLoop = true);

	void init();	// コンストラクタ内で呼び出している
	void update();
	void draw(Light* light);

	void drawWithUpdate(Light* light);

	static void startDraw();

#pragma region アクセッサ

	inline const FbxModel* getModel() const { return model; }
	inline void setModel(FbxModel* model) { this->model = model; }

	inline const DirectX::XMFLOAT3& getScale() const { return scale; }
	inline void setScale(const XMFLOAT3& scale) { this->scale = scale; }

	inline const DirectX::XMFLOAT3& getPosition() const { return position; }
	inline void setPosition(const XMFLOAT3& position) { this->position = position; }

	inline const DirectX::XMFLOAT3& getRotation() const { return rotation; }
	inline void setRotation(const XMFLOAT3& rotation) { this->rotation = rotation; }

	// fbxとobj両方の親がいる場合、fbxが優先される
	inline const FbxObj3d* getParent() const { return parent; }
	// fbxとobj両方の親がいる場合、fbxが優先される
	inline const Object3d* getObjParent() const { return objParent; }

	// fbxとobj両方の親がいる場合、fbxが優先される
	inline void setParent(FbxObj3d* parent) { this->parent = parent; }
	// fbxとobj両方の親がいる場合、fbxが優先される
	inline void setObjParent(Object3d* objParent) { this->objParent = objParent; }

	inline void setCamera(Camera* camera) { this->camera = camera; }

#pragma endregion アクセッサ

	void playAnimation();
	void stopAnimation(bool resetPoseFlag = true);

	DirectX::XMFLOAT3 calcVertPos(size_t vertNum);

	DirectX::XMFLOAT2 calcScreenPos();

protected:
	Camera* camera = nullptr;

	FbxObj3d* parent = nullptr;
	Object3d* objParent = nullptr;

	ComPtr<ID3D12Resource> constBuffTransform;

	XMFLOAT4 color = { 1, 1, 1, 1 };

	XMFLOAT3 scale = { 1, 1, 1 };
	XMFLOAT3 rotation = { 0, 0, 0 };
	XMFLOAT3 position = { 0, 0, 0 };
	XMMATRIX matWorld{};
	FbxModel* model = nullptr;

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
};
