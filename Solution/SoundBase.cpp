#include "SoundBase.h"
#include <cassert>


SoundBase::SoundBase() {
	// XAudio�G���W���̃C���X�^���X�𐶐�
	HRESULT result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// �}�X�^�[�{�C�X�𐶐�
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
}

SoundBase::~SoundBase() {
	this->xAudio2.Reset();
}