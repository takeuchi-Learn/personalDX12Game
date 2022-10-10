#include "PostEffect.hlsli"

Texture2D<float4> tex0 : register(t0); // 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> tex1 : register(t1); // 1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp : register(s0); // 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

float fracNoise(float2 coord)
{
	return frac(sin(dot(coord, float2(13.f, 80.f))) * 44000.f);
}

float vignatte(float2 uv)
{
	// UV�l�̒��S����̋���(�傫��)
	float len = distance(uv, 0.5f);

	return -len * vignIntensity;
}

float noise(float2 uv, float time)
{
	float noiseNum = fracNoise(uv * time * 10.f) - 0.5f;
	return noiseNum * noizeIntensity;
}

float speedLine(float2 uv, float seed, float colourIntensity = 0.125f)
{
	// uv���W��-1 ~ +1�ɕϊ�
	float2 pos = (uv - 0.5f) * 2.f;

	// 0 ~ 1�̊p�x
	float angle = ((atan2(pos.r, pos.g) / 3.141592653589793f) + 1.f) / 2.f;

	// �p�x�̒l��i�K�I�ɂ���
	static float divNum = 1024;
	float floorAngle = floor(angle * divNum) / divNum * seed;

	// (�i�K�I��)�p�x���Q�l�Ƀm�C�Y��Ԃ�
	return saturate(fracNoise(float2(floorAngle, floorAngle))) * colourIntensity;
}

float4 main(VSOutput input) : SV_TARGET
{
	static float PI = 3.141592653589793f;
	static float PI2 = 6.283185307179586f;
	
	// --------------------
	// ���U�C�N
	// --------------------
	
	// ��ʂ����E���ꂼ��mosaicNum���������傫���̃��U�C�N�ɂȂ�
	float2 uv = floor(input.uv * mosaicNum) / mosaicNum;
	// ����[s]
	float time = nowTime / oneSec;

	// --------------------
	// �r�l�b�^
	// --------------------
	float vignNum = vignatte(uv);

	// --------------------
	// �������̂悤�Ȃ���
	// --------------------
	static float slnSpeed = 8.f;
	static float slnDivLevel = 96.f;
	static float slnPower = 24.f;
	float sinNum = uv.y * slnDivLevel + time * slnSpeed;
	float sLineNum = fracNoise(float2(time, uv.y)) * sin(sinNum) * sin(sinNum + 0.75f) + 1.f;
	sLineNum /= -slnPower;

	// --------------------
	// ������
	// --------------------
	static float slSpeed = 1.f / 4.f;
	static float slSize = 1.f / 64.f;
	static float slPower = 1.f / 16.f;
	float sbTime = frac(time * slSpeed);
	float seTime = sbTime + slSize;
	
	float2 slUv = float2(
		uv.x + sin(smoothstep(sbTime, seTime, uv.y) *
				   PI2) * slPower,
		uv.y
	);
	uv = slUv;

	// --------------------
	// rgb���炵
	// --------------------
	float4 texColor0 = tex0.Sample(smp, uv);
	texColor0.g = tex0.Sample(smp, uv + rgbShiftNum).g;
	static float gamma = 1.f / 2.2f;
	texColor0 = pow(texColor0, gamma);

	float noiseNum = noise(input.uv, time);
	
	// --------------------
	// �W����
	// --------------------
	
	// ���Ԃœ����Ăق����̂�seed�ɂ͎��Ԃ�����
	float speedLineNum = speedLine(input.uv, fmod(nowTime / oneSec, 1.f), speedLineIntensity);
	// ���S�ɋ߂��قǐF�𔖂�����
	speedLineNum *= distance(float2(0.5f, 0.5f), input.uv);

	float4 drawCol = float4(texColor0.rgb + sLineNum + vignNum + noiseNum + speedLineNum, alpha);

	return drawCol;
}
