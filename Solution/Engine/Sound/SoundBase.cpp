#include "SoundBase.h"
#include <cassert>

#pragma comment(lib,"xaudio2.lib")

SoundBase::SoundBase()
{
	// XAudioエンジンのインスタンスを生成
	HRESULT result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスを生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
}

SoundBase::~SoundBase()
{
	masterVoice->DestroyVoice();
	this->xAudio2.Reset();
}