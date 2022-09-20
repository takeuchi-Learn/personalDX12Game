#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <wrl.h>

class Sound
{
public:
#pragma region チャンク
	// チャンクヘッダ
	struct ChunkHeader
	{
		char id[4]; // チャンク毎のID
		int32_t size;  // チャンクサイズ
	};

	// RIFFヘッダチャンク
	struct RiffHeader
	{
		ChunkHeader chunk;   // "RIFF"
		char type[4]; // "WAVE"
	};

	// FMTチャンク
	struct FormatChunk
	{
		ChunkHeader chunk; // "fmt "
		WAVEFORMATEX fmt; // 波形フォーマット
	};
#pragma endregion

	// --------------------
	// メンバ変数
	// --------------------
private:
	//波形フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* pBuffer;
	//バッファのサイズ
	unsigned int bufferSize;

	IXAudio2SourceVoice* pSourceVoice = nullptr;

	// --------------------
	// メンバ関数
	// --------------------
public:
	// 音声データの読み込み
	Sound(const char* filename);

	// 音声データの解放
	~Sound();

	// --------------------
	// static関数
	// --------------------
private:
	static void createSourceVoice(Sound* soundData);

public:
	// 音声再生停止
	static void SoundStopWave(Sound* soundData);

	/// @brief 音声再生
	/// @param soundData 再生するサウンドデータオブジェクト
	/// @param loopCount 0で繰り返し無し、XAUDIO2_LOOP_INFINITEで永遠
	/// @param volume 0 ~ 1
	static void SoundPlayWave(Sound* soundData,
							  int loopCount = 0, float volume = 0.2);

	//再生状態の確認
	static bool checkPlaySound(Sound* soundData);
};
