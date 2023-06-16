#pragma once

#include "Sprite.h"
#include <string>

#include <DirectXMath.h>

class DebugText
{
public: // 定数の宣言
	static const int maxCharCount = 256;    // 最大文字数
	// 画像サイズに合わせて変えたい(fontWidth, fontHeight)
	static const int fontWidth = 9 * 2;         // フォント画像内1文字分の横幅
	static const int fontHeight = 18 * 2;       // フォント画像内1文字分の縦幅
	static const int fontLineCount = 14;    // フォント画像内1行分の文字数

public:
	uint8_t tabSize = 4;	// 初期値はSPACE4つ分

private:
	// メンバ関数
	void Initialize(UINT texnumber, const SpriteBase* spriteCommon, uint8_t tabSIze = 4);

public:
	// 内部でinitializeを呼び出している
	DebugText(UINT texNum, const SpriteBase* spriteCommon, uint8_t tabSize = 4);

	// ￥n : X座標をして位置に戻し、Y座標を文字の高さ分加算する
	// ￥t : tabSize文字分右にずらす
	void Print(const SpriteBase* spriteCommon, const std::string& text,
			   const float x, const float y, const float scale = 1.0f,
			   DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1, 1, 1, 1));

	// 内部でvsnprintfを使用
	// @return vsnprintfの戻り値
	int formatPrint(const SpriteBase* spriteCommon, const float x, const float y, const float scale, DirectX::XMFLOAT4 color, const char* fmt, ...);

	void DrawAll(DX12Base* dxBase, const SpriteBase* spriteCommon);

private: // メンバ変数
	// スプライトデータの配列
	Sprite sprites[maxCharCount];
	// スプライトデータ配列の添え字番号
	int spriteIndex = 0;
};