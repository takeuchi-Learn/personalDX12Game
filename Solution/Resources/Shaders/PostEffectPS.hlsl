#include "PostEffect.hlsli"

Texture2D<float4> tex0 : register(t0);   // 0番スロットに設定されたテクスチャ
Texture2D<float4> tex1 : register(t1);   // 1番スロットに設定されたテクスチャ
SamplerState smp : register(s0);         // 0番スロットに設定されたサンプラー

float fracNoise(float2 coord) {
	return frac(sin(dot(coord, float2(13.f, 80.f)) + 0.f) * 44000.f);
}

float vignatte(float2 uv) {
	// UV値の中心からの距離(大きさ)
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
	
	// 画面を左右それぞれmosaicNum分割した大きさのモザイクになる
	float2 uv = floor(input.uv * mosaicNum) / mosaicNum;
	// 時間[s]
	float time = nowTime / oneSec;

	float vignNum = vignatte(uv);

	// 走査線のようなもの
	#define slnSpeed  (8.f)
	#define slnDivLevel (96.f)
	#define slnPower (24.f)
	float sinNum = uv.y * slnDivLevel + time * slnSpeed;
	float sLineNum = fracNoise(float2(time, uv.y)) * sin(sinNum) * sin(sinNum + 0.75f) + 1;
	sLineNum /= -slnPower;

	// 走査線
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

	// rgbずらし
	float4 texColor0;
	texColor0.g = tex0.Sample(smp, uv + rgbShiftNum).g;
	texColor0.rba = tex0.Sample(smp, uv).rba;

	float noiseNum = noise(input.uv, time);

	float4 drawCol = float4(texColor0.rgb + sLineNum + vignNum + noiseNum, alpha);

	return drawCol;
}
