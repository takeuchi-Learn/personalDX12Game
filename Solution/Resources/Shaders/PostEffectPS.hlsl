#include "PostEffect.hlsli"

Texture2D<float4> tex0 : register(t0);   // 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> tex1 : register(t1);   // 1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp : register(s0);         // 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

float fracNoise(float2 coord) {
	return frac(sin(dot(coord, float2(13.f, 80.f)) + 0.f) * 44000.f);
}

float vignatte(float2 uv) {
	// UV�l�̒��S����̋���(�傫��)
	float len = distance(uv, 0.5f);

	return -len * vignIntensity;
}

float noise(float2 uv, float time) {
	float noiseNum = fracNoise(uv * time * 10.f) - 0.5f;
	return noiseNum * noizeIntensity;
}

float4 main(VSOutput input) : SV_TARGET
{
	#define PI (3.141592653589793f)
	#define PI2 (6.283185307179586f)
	
	// ��ʂ����E���ꂼ��mosaicNum���������傫���̃��U�C�N�ɂȂ�
	float2 uv = floor(input.uv * mosaicNum) / mosaicNum;
	// ����[s]
	float time = nowTime / oneSec;

	float vignNum = vignatte(uv);

	// �������̂悤�Ȃ���
	#define slnSpeed  (8.f)
	#define slnDivLevel (96.f)
	#define slnPower (24.f)
	float sinNum = uv.y * slnDivLevel + time * slnSpeed;
	float sLineNum = fracNoise(float2(time, uv.y)) * sin(sinNum) * sin(sinNum + 0.75f) + 1;
	sLineNum /= -slnPower;

	// ������
	#define slSpeed (0.25f)
	#define slSize (0.03125f)
	#define slPower (0.0625f)
	float sbTime = frac(time * slSpeed);
	float seTime = sbTime + slSize;
	
	float2 slUv = float2(
		uv.x + sin(smoothstep(sbTime, seTime, uv.y) *
				   PI2) * slPower,
		uv.y
	);
	uv = slUv;

	// rgb���炵
	float shiftRaito = fmod(time, 1.f);
	float rgbUvNum = 0.005f * sin(shiftRaito * PI2);
	float4 texColor0;
	texColor0.r = tex0.Sample(smp, uv + float2(rgbUvNum, 0.f)).r;
	texColor0.gba = tex0.Sample(smp, uv).gba;
	float4 texColor1 = tex1.Sample(smp, uv);
	texColor1.r = tex1.Sample(smp, uv + float2(rgbUvNum, 0.f)).r;

	// tex0��tex1�̉��ȂɂȂ�悤�ɕ`��
	float4 col = lerp(texColor0, texColor1, step(0.05f, fmod(input.uv.y, 0.1f)));

	float noiseNum = noise(input.uv, time);

	float4 drawCol = float4(col.rgb + sLineNum + vignNum + noiseNum, alpha);

	return drawCol;
}
