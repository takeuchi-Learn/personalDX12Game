#include "Sound.h"

#include "SoundBase.h"

#include <fstream>
#include <cassert>

Sound::Sound(const char* filename)
{
	//1.ファイルオープン
	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	//ファイルオープン失敗を検出する
	assert(file.is_open());

	//2.wavデータ読み込み
	//RIFFヘッダーの読み込み
	RiffHeader riff{};
	file.read((char*)&riff, sizeof(riff));
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}
	//タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0)
	{
		assert(0);
	}
	//Formatチャンクの読み込み
	FormatChunk format = {};
	//チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0)
	{
		assert(0);
	}
	//チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);
	//Dataチャンクの読み込み
	ChunkHeader data{};
	file.read((char*)&data, sizeof(data));
	//JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0)
	{
		//読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		//再読み込み
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0)
	{
		assert(0);
	}
	//Dataチャンクデータの一部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//3.ファイルクローズ
	//Waveファイルを閉じる
	file.close();

	//4.読み込んだ音声データを格納

	this->wfex = format.fmt;
	this->pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	this->bufferSize = data.size;

	createSourceVoice(this);
}

Sound::~Sound()
{
	SoundStopWave(this);

	if (this->pSourceVoice != nullptr)
	{
		this->pSourceVoice->DestroyVoice();
	}

	//バッファのメモリを解放
	delete[] this->pBuffer;

	this->pBuffer = 0;
	this->bufferSize = 0;
	this->wfex = {};
}

void Sound::createSourceVoice(Sound* soundData)
{
	//波形フォーマットをもとにSourceVoiceの生成
	HRESULT result = SoundBase::getInstange()->xAudio2->CreateSourceVoice(&soundData->pSourceVoice, &soundData->wfex);
	assert(SUCCEEDED(result));
}

void Sound::SoundStopWave(Sound* soundData)
{
	HRESULT result = soundData->pSourceVoice->Stop();
	assert(SUCCEEDED(result));

	result = soundData->pSourceVoice->FlushSourceBuffers();
	assert(SUCCEEDED(result));

	/*XAUDIO2_BUFFER buf{};
	result = soundData.pSourceVoice->SubmitSourceBuffer(&buf);*/
}

void Sound::SoundPlayWave(Sound* soundData, int loopCount, float volume)
{
	//波形フォーマットをもとにSourceVoiceの生成
	createSourceVoice(soundData);

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData->pBuffer;
	buf.AudioBytes = soundData->bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.LoopCount = loopCount;

	//波形データの再生
	HRESULT result = soundData->pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = soundData->pSourceVoice->SetVolume(volume);
	assert(SUCCEEDED(result));

	result = soundData->pSourceVoice->Start();
}

bool Sound::checkPlaySound(Sound* soundData)
{
	if (soundData == nullptr ||
		soundData->pSourceVoice == nullptr) return false;

	XAUDIO2_VOICE_STATE tmp{};
	soundData->pSourceVoice->GetState(&tmp);
	if (tmp.BuffersQueued == 0U)
	{
		return false;
	}
	return true;
}