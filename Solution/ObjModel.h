#pragma once

#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include "Mesh.h"

class ObjModel {
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

	// --------------------
	// static
	// --------------------
public:

private:
	static ID3D12Device* dev;
	// デスクリプタサイズ
	static UINT descriptorHandleIncrementSize;

	// --------------------
	// メンバ変数
	// --------------------
private:
	std::string name;
	std::vector<Mesh*> meshes;
	std::unordered_map<std::string, Material*> materials;
	// デフォルトマテリアル
	Material* defaultMaterial = nullptr;
	// デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeap;

	/// <summary>
	/// マテリアル読み込み
	/// </summary>
	void loadMaterial(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// マテリアル登録
	/// </summary>
	void addMaterial(Material* material);

	/// <summary>
	/// デスクリプタヒープの生成
	/// </summary>
	void createDescriptorHeap();

	/// <summary>
	/// テクスチャ読み込み
	/// </summary>
	void loadTextures(const std::string& dirPath, UINT texNum);

public:
	// ----------
	// static
	// ----------

	// 静的初期化
	static void staticInit(ID3D12Device* device);


	// メンバ

	// @param dirPath : objファイルのある場所のパス(例 : Resources/player/)
	// @param objModelName : objファイルのファイル名(拡張子なし。例 : player.obj -> player)
	ObjModel(const std::string& dirPath, const std::string& objModelName, UINT texNum = 0u, bool smoothing = false);
	~ObjModel();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dirPath">モデルファイルのあるパス</param>
	/// <param name="modelname">モデル名(例 : Resources/player.obj)</param>
	void init(const std::string& dirPath, const std::string& modelname, UINT texNum = 0u, bool smoothing = false);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="cmdList">命令発行先コマンドリスト</param>
	void draw(ID3D12GraphicsCommandList* cmdList);
};

