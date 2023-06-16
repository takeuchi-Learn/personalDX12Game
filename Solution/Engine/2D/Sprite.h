#pragma once
#include "SpriteBase.h"

#include "System/DX12Base.h"

class Sprite
{
private:
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMMATRIX = DirectX::XMMATRIX;
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	// 定数バッファ用データ構造体
	struct ConstBufferData
	{
		XMFLOAT4 color; // 色 (RGBA)
		XMMATRIX mat;   // ３Ｄ変換行列
	};

	// 頂点データ
	struct VertexPosUv
	{
		XMFLOAT3 pos; // xyz座標
		XMFLOAT2 uv;  // uv座標
	};

	// --------------------
	// スプライト1枚分のデータ
	// --------------------
protected:
	//頂点バッファ;
	ComPtr<ID3D12Resource> vertBuff;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//定数バッファ;
	ComPtr<ID3D12Resource> constBuff;
	// ワールド行列
	XMMATRIX matWorld;

public:
	// Z軸回りの回転角
	float rotation = 0.0f;
	// 座標
	XMFLOAT3 position = { 0,0,0 };
	// 色(RGBA)
	XMFLOAT4 color = { 1, 1, 1, 1 };
	// 非表示
	bool isInvisible = false;

private:
	// テクスチャ番号
	UINT texNumber = 0;
	// 大きさ
	XMFLOAT2 size = { 100, 100 };
	// アンカーポイント
	XMFLOAT2 anchorpoint = { 0.5f, 0.5f };
	// 左右反転
	bool isFlipX = false;
	// 上下反転
	bool isFlipY = false;
	// テクスチャ左上座標
	XMFLOAT2 texLeftTop = { 0, 0 };
	// テクスチャ切り出しサイズ
	XMFLOAT2 texSize = { 100, 100 };

	bool dirty = false;

public:
	// --------------------
	// アクセッサ
	// --------------------
	inline void setAnchorPoint(const XMFLOAT2& ap) { anchorpoint = ap; dirty = true; }
	inline const XMFLOAT2& getAnchorPoint() const { return anchorpoint; }

	inline void setSize(const XMFLOAT2& size) { this->size = size; dirty = true; }
	inline const XMFLOAT2& getSize() const { return size; }

	inline void setTexLeftTop(const XMFLOAT2& texLT) { texLeftTop = texLT; dirty = true; }
	inline const XMFLOAT2& getTexLeftTop() const { return texLeftTop; }

	inline void setTexNum(const UINT& texNum) { texNumber = texNum; dirty = true; }
	inline UINT getTexNum() const { return texNumber; }

	inline void setTexSize(const XMFLOAT2& texSize) { this->texSize = texSize; dirty = true; }
	inline const XMFLOAT2& getTexSize() const { return texSize; }

	inline void setFlipX(const bool isFlipX) { this->isFlipX = isFlipX; dirty = true; }
	inline bool getFlipX() const { return isFlipX; }
	inline void setFlipY(const bool isFlipY) { this->isFlipY = isFlipY; dirty = true; }
	inline bool getFlipY() const { return isFlipY; }

public:
	// --------------------
	// 個別
	// --------------------
protected:
	// スプライト単体頂点バッファの転送
	void SpriteTransferVertexBuffer(const SpriteBase* spriteCommon);

	// スプライト生成
	void create(ID3D12Device* dev, int window_width, int window_height,
				UINT texNumber, const SpriteBase* spriteCommon, XMFLOAT2 anchorpoint = { 0.5f,0.5f },
				bool isFlipX = false, bool isFlipY = false);

public:
	// 初期化なし
	Sprite() {};
	// 初期化有り(create関数の呼び出し)
	Sprite(UINT texNumber,
		   const SpriteBase* spriteCommon,
		   XMFLOAT2 anchorpoint = { 0.5f,0.5f },
		   bool isFlipX = false, bool isFlipY = false);

	// スプライト単体更新
	void update(const SpriteBase* spriteCommon);

	// スプライト単体描画
	void draw(ID3D12GraphicsCommandList* cmdList, const SpriteBase* spriteCommon, ID3D12Device* dev);

	// 更新と描画を同時に行う
	void drawWithUpdate(DX12Base* dxCom, const SpriteBase* spriteCommon);
};