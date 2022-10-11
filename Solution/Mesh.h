#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include "Material.h"
#include <vector>

#include <unordered_map>

/// @brief 形状データ
class Mesh
{
private: // エイリアス
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public: // サブクラス
	// 頂点データ構造体（テクスチャあり）
	struct VertexPosNormalUv
	{
		XMFLOAT3 pos; // xyz座標
		XMFLOAT3 normal; // 法線ベクトル
		XMFLOAT2 uv;  // uv座標
	};

public: // 静的メンバ関数

	/// @brief 静的初期化
	/// @param dev デバイス
	static void staticInit(ID3D12Device* dev);

private: // 静的メンバ変数
	// デバイス
	static ID3D12Device* dev;

public: // メンバ関数

	/// @brief verticesのゲッター
	inline const std::vector<VertexPosNormalUv>& getVertices() const { return vertices; }

	/// @brief indicesのゲッター
	inline const std::vector<unsigned short>& getIndices() const { return indices; }

	/// @brief 名前を取得
	/// @return 名前
	const std::string& getName() { return name; }

	/// @brief 名前をセット
	/// @param name 名前
	void setName(const std::string& name);

	/// @brief 頂点データの追加
	/// @param vertex 頂点データ
	void addVertex(const VertexPosNormalUv& vertex);

	/// @brief 頂点インデックスの追加
	/// @param vertex 頂点インデックス
	void addIndex(unsigned short index);

	/// @brief 頂点データの数を取得
	/// @return 頂点データの数
	inline size_t getVertexCount() { return vertices.size(); }

	/// @brief マテリアルの取得
	/// @return マテリアル
	Material* getMaterial() { return material; }

	/// @brief マテリアルの割り当て
	/// @param material	マテリアル
	void setMaterial(Material* material);

	/// @brief バッファの生成
	void createBuffers();

	/// @brief 頂点バッファを取得
	/// @return 頂点バッファ
	const D3D12_VERTEX_BUFFER_VIEW& getVBView() { return vbView; }

	/// @brief インデックスバッファを取得
	/// @return インデックスバッファ
	const D3D12_INDEX_BUFFER_VIEW& getIBView() { return ibView; }

	/// @brief 描画
	/// @param cmdList 命令発行先コマンドリスト
	void draw(ID3D12GraphicsCommandList* cmdList);

	/// @brief エッジ平滑化データ追加
	/// @param indexPosition 座標インデックス
	/// @param indexVertex 頂点インデックス
	void addSmoothData(unsigned short indexPosition, unsigned short indexVertex);

	// 平滑化された頂点法線の計算
	void calculateSmoothedVertexNormals();

private: // メンバ変数
	// 名前
	std::string name;
	// 頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	// インデックスバッファ
	ComPtr<ID3D12Resource> indexBuff;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView{};
	// 頂点データ配列
	std::vector<VertexPosNormalUv> vertices;
	// 頂点インデックス配列
	std::vector<unsigned short> indices;
	// マテリアル
	Material* material = nullptr;
	std::unordered_map<unsigned short, std::vector<unsigned short>> smoothData;
};
