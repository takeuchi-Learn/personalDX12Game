#include "SoundBase.h"
#include <cassert>


SoundBase::SoundBase() {
	// XAudioエンジンのインスタンスを生成
	HRESULT result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスを生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
}

SoundBase::~SoundBase() {
	this->xAudio2.Reset();
}