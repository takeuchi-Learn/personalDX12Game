#pragma once

#include "fbxsdk.h"

#include <d3d12.h>
#include <d3dx12.h>

#include <string>

#include "FbxModel.h"

class FbxLoader {
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>インスタンス</returns>
	static FbxLoader* GetInstance();

private:
	// privateなコンストラクタ（シングルトンパターン）
	FbxLoader() = default;
	// privateなデストラクタ（シングルトンパターン）
	~FbxLoader() = default;
	// コピーコンストラクタを禁止（シングルトンパターン）
	FbxLoader(const FbxLoader& obj) = delete;
	// コピー代入演算子を禁止（シングルトンパターン）
	void operator=(const FbxLoader& obj) = delete;

public:
	// モデルを格納するルートパス
	static const std::string baseDir;

	static void convertMatrixFromFbx(DirectX::XMMATRIX* dst,
									 const FbxAMatrix& src);

	// 初期化
	void init(ID3D12Device* dev);

	void fin();

	// @param modelName ファイル名の拡張子を含まないもの
	FbxModel* loadModelFromFile(const std::string& modelName);

private:
	ID3D12Device* dev = nullptr;
	FbxManager* fbxManager = nullptr;
	FbxImporter* fbxImporter = nullptr;

	// テクスチャがない場合の標準テクスチャファイル名
	static const std::string defaultTextureFileName;

	/// <summary>
	/// 再帰的にノード構成を解析
	/// </summary>
	/// <param name="model">読み込み先のモデルオブジェクト</param>
	/// <param name="fbxNode">解析対象のノード</param>
	void parseNodeRecursive(FbxModel* model, FbxNode* fbxNode, FbxModel::Node* parent = nullptr);

	/// <summary>
	/// メッシュの読み取り
	/// </summary>
	/// <param name="model">読み込み先のモデルオブジェクト</param>
	/// <param name="fbxNode">解析対象のノード</param>
	void parseMesh(FbxModel* model, FbxNode* fbxNode);

	// 頂点座標
	void parseMeshVertices(FbxModel* model, FbxMesh* fbxMesh);

	// 面情報
	void parseMeshFaces(FbxModel* model, FbxMesh* fbxMesh);

	// マテリアル
	void parseMaterial(FbxModel* model, FbxNode* fbxNode);

	// テクスチャ
	void loadTexture(FbxModel* model, const std::string& fullPath);

	std::string ExtractFileName(const std::string& path);

	void parseSkin(FbxModel* model, FbxMesh* fbxMesh);
};