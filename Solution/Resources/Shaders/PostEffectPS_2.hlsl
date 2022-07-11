#include "PostEffect.hlsli"

Texture2D<float4> tex0 : register(t0);   // 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> tex1 : register(t1);   // 1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp : register(s0);         // 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

float4 main(VSOutput input) : SV_TARGET
{
	float4 texColor0 = tex0.Sample(smp, input.uv);
	float4 texColor1 = tex1.Sample(smp, input.uv);

	// tex0��tex1�̉��ȂɂȂ�悤�ɕ`��
	float4 col = lerp(texColor0, texColor1, step(0.05f, fmod(input.uv.y, 0.1f)));

	return col;
}
