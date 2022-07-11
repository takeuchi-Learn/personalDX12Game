#pragma once

#include <string>

#include <DirectXMath.h>

#include <vector>

#include <DirectXTex.h>

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>

#include <fbxsdk.h>

class FbxModel {
public:
	friend class FbxLoader;

private:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	using TexMetadata = DirectX::TexMetadata;
	using ScratchImage = DirectX::ScratchImage;
	using string = std::string;
	template <class T> using vector = std::vector<T>;

public:



	// ボーンインデックスの最大数
	static const int MAX_BONE_INDICES = 4;



	struct Node {
		std::string name;
		// ローカルスケール
		DirectX::XMVECTOR scaling = { 1,1,1,0 };
		// ローカル回転角
		DirectX::XMVECTOR rotation = { 0,0,0,0 };
		// ローカル移動
		DirectX::XMVECTOR translation = { 0,0,0,1 };
		// ローカル変形行列
		DirectX::XMMATRIX transform{};
		// グローバル変形行列
		DirectX::XMMATRIX globalTransform{};
		// 親ノード
		Node* parent = nullptr;
	};

	struct VertexPosNormalUvSkin {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		UINT boneIndex[MAX_BONE_INDICES];	// ボーン番号
		float boneWeight[MAX_BONE_INDICES];	// 重み
	};

	// ボーンの構造体
	struct Bone {
		// 名前
		std::string name{};
		// 初期姿勢の逆行列
		DirectX::XMMATRIX invInitialPose{};
		// クラスター(FBX側のボーン情報)
		FbxCluster* fbxCluster = nullptr;
		// コンストラクタ
		Bone(const std::string& name) : name(name) {};
	};

	// 定数バッファ用データ構造体B1
	struct ConstBufferDataB1 {
		DirectX::XMFLOAT3 ambient;	// アンビエント
		float pad1;		// パディング
		DirectX::XMFLOAT3 diffuse;	// ディフューズ
		float pad2;		// パディング
		DirectX::XMFLOAT3 specular;	// スペキュラー
		float alpha;	// アルファ
	};

private:

	std::string name;
	std::vector<Node> nodes;

	// メッシュを持つノード
	Node* meshNode = nullptr;

	// 頂点データ
	std::vector<VertexPosNormalUvSkin> vertices;

	// 頂点インデックス
	std::vector<unsigned int> indices;

	DirectX::XMFLOAT3 ambient = { 0.5f, 0.5f, 0.5f };
	DirectX::XMFLOAT3 diffuse = { 0.6f, 0.6f, 0.6f };
	DirectX::XMFLOAT3 specular = { 0.8f, 0.8f, 0.8f };
	float alpha = 1.f;
	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratchImg = {};

	ComPtr<ID3D12Resource> vertBuff;
	ComPtr<ID3D12Resource> indexBuff;
	ComPtr<ID3D12Resource> texBuff;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ComPtr<ID3D12DescriptorHeap> descHeapSRV;

	// ボーン配列
	std::vector<Bone> bones;


	// 定数バッファ
	ComPtr<ID3D12Resource> constBuffB1;

	// FBXシーン
	FbxScene* fbxScene = nullptr;

	bool materialDirty = true;

	void createConstBuffB1();

	void transferConstBuffB1();

public:
	inline void setAmbient(const DirectX::XMFLOAT3 &ambient) { this->ambient = ambient, materialDirty = true; }
	inline void setDiffuse(const DirectX::XMFLOAT3 &diffuse) { this->diffuse = diffuse, materialDirty = true; }
	inline void setSpecular(const DirectX::XMFLOAT3 &specular) { this->specular = specular, materialDirty = true; }
	inline void setAlpha(float alpha) { this->alpha = alpha; }

	inline auto getSpecular() { return specular; }

	inline ID3D12Resource* getConstBuffB1() { return constBuffB1.Get(); }

	FbxScene* getFbxScene() { return fbxScene; }

	FbxModel();
	~FbxModel();

	void createBuffers(ID3D12Device* dev);

	void draw(ID3D12GraphicsCommandList* cmdList);

	const XMMATRIX& GetModelTransform() { return meshNode->globalTransform; }

	std::vector<Bone>& getBones() { return bones; }
};

