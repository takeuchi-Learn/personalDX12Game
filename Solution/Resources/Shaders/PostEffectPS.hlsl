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
	// 画面を左右それぞれmosaicNum分割した大きさのモザイクになる
	float2 uv = floor(input.uv * mosaicNum) / mosaicNum;
	// 時間[s]
	float time = nowTime / oneSec;

	float vignNum = vignatte(uv);

	// 走査線のようなもの
	float sSpeed = 8.f;
	float sDivLevel = 96.f;
	float sPower = 24.f;
	float sinNum = uv.y * sDivLevel + time * sSpeed;
	float sLineNum = fracNoise(float2(time, uv.y)) * sin(sinNum) * sin(sinNum + 0.75f) + 1;
	sLineNum /= -sPower;

	// 走査線
	float slSpeed = 0.25f;
	float slSize = 0.03125f;
	float slPower = 0.0625f;
	float sbTime = frac(time * slSpeed);
	float seTime = sbTime + slSize;

	// 6.28318f = 2PI
	float2 slUv = float2(
		uv.x + sin(smoothstep(sbTime, seTime, uv.y) *
				   6.28318f) * slPower,
		uv.y
	);
	uv = slUv;

	// rgbずらし
	float rgbUvNum = 0.005f * sin(time * 3.141592653589793f);
	float4 texColor0 = tex0.Sample(smp, uv);
	texColor0.r = tex0.Sample(smp, uv + float2(rgbUvNum, 0.f)).r;
	float4 texColor1 = tex1.Sample(smp, uv);
	texColor1.r = tex1.Sample(smp, uv + float2(rgbUvNum, 0.f)).r;

	// tex0とtex1の横縞になるように描画
	float4 col = lerp(texColor0, texColor1, step(0.05f, fmod(input.uv.y, 0.1f)));

	float noiseNum = noise(input.uv, time);

	float4 drawCol = float4(col.rgb + sLineNum + vignNum + noiseNum, alpha);

	return drawCol;
}
