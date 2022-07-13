#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <wrl.h>
class SoundBase {

	class XAudio2VoiceCallback : public IXAudio2VoiceCallback {
	public:
		// ボイス処理パスの開始時
		STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) {};
		// ボイス処理パスの終了時
		STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) {};
		// バッファストリームの再生が終了した時
		STDMETHOD_(void, OnStreamEnd) (THIS) {};
		// バッファの使用開始時
		STDMETHOD_(void, OnBufferStart) (THIS_ void *pBufferContext) {};
		// バッファの末尾に達した時
		STDMETHOD_(void, OnBufferEnd) (THIS_ void *pBufferContext) {};
		// 再生がループ位置に達した時
		STDMETHOD_(void, OnLoopEnd) (THIS_ void *pBufferContext) {};
		// ボイスの実行エラー時
		STDMETHOD_(void, OnVoiceError) (THIS_ void *pBufferContext, HRESULT Error) {};
	};
public:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice *masterVoice;
	XAudio2VoiceCallback voiceCallback;

	SoundBase();
	~SoundBase();
};

