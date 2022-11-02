#include "DebugText.h"

#include <DirectXMath.h>

DebugText::DebugText(UINT texNum, const SpriteBase* spriteCommon, uint8_t tabSize)
{
	Initialize(texNum, spriteCommon, tabSize);
}

void DebugText::Initialize(UINT texnumber, const SpriteBase* spriteCommon,
						   uint8_t tabSize)
{
	this->tabSize = tabSize;
	// 全てのスプライトデータについて
	for (UINT i = 0, len = _countof(sprites); i < len; ++i)
	{
		// スプライトを生成する
		sprites[i] = Sprite(texnumber, spriteCommon, { 0, 0 });
	}
}

void DebugText::Print(const SpriteBase* spriteCommon, const std::string& text,
					  const float x, const float y, const float scale,
					  DirectX::XMFLOAT4 color)
{
	std::string textLocal = text;

	int posNumX = 0, posNumY = 0;

	// 全ての文字について
	for (UINT i = 0, len = (UINT)text.size(); i < len; ++i, ++posNumX)
	{
		// 最大文字数超過
		if (spriteIndex >= maxCharCount)
		{
			break;
		}

		auto drawCol = color;

		if (i < maxCharCount - 1)
		{
			if (strncmp(&textLocal[i], "\n", 1) == 0)
			{
				posNumX = -1;
				posNumY++;
				textLocal[i] = ' ';
				drawCol.w = 0.f;
			}  if (strncmp(&textLocal[i], "\t", 1) == 0)
			{
				posNumX += (int)tabSize - 1;
				textLocal[i] = ' ';
				drawCol.w = 0.f;
			}
		}

		// 1文字取り出す(※ASCIIコードでしか成り立たない)
		const unsigned char& character = textLocal[i];

		// ASCIIコードの2段分飛ばした番号を計算
		int fontIndex = character - 32;
		if (character >= 0x7f)
		{
			fontIndex = 0;
		}

		int fontIndexY = fontIndex / fontLineCount;
		int fontIndexX = fontIndex % fontLineCount;

		// 座標計算
		sprites[spriteIndex].position = { x + fontWidth * scale * posNumX, y + fontHeight * scale * posNumY, 0 };
		sprites[spriteIndex].color = drawCol;
		sprites[spriteIndex].setTexLeftTop({ (float)fontIndexX * fontWidth, (float)fontIndexY * fontHeight });
		sprites[spriteIndex].setTexSize({ fontWidth, fontHeight });
		sprites[spriteIndex].setSize({ fontWidth * scale, fontHeight * scale });
		// 更新
		sprites[spriteIndex].update(spriteCommon);

		// 文字を１つ進める
		spriteIndex++;
	}
}

int DebugText::formatPrint(const SpriteBase* spriteCommon,
						   const float x, const float y, const float scale,
						   DirectX::XMFLOAT4 color, const char* fmt, ...)
{
	char outStrChar[maxCharCount]{};

	constexpr size_t bufferCount = size_t(maxCharCount - 1);

	va_list args;

	va_start(args, fmt);
	const int ret = vsnprintf(outStrChar, bufferCount, fmt, args);

	Print(spriteCommon, outStrChar, x, y, scale, color);
	va_end(args);

	return ret;
}

// まとめて描画
void DebugText::DrawAll(DX12Base* dxBase, const SpriteBase* spriteCommon)
{
	// 全ての文字のスプライトについて
	for (UINT i = 0; i < (UINT)spriteIndex; ++i)
	{
		// スプライト描画
		sprites[i].draw(dxBase->getCmdList(), spriteCommon, dxBase->getDev());
	}

	spriteIndex = 0;
}