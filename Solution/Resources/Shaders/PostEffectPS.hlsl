#include "PostEffect.hlsli"

Texture2D<float4> tex0 : register(t0); // 0番スロットに設定されたテクスチャ
Texture2D<float4> tex1 : register(t1); // 1番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

float fracNoise(float2 coord)
{
	return frac(sin(dot(coord, float2(13.f, 80.f))) * 44000.f);
}

float vignatte(float2 uv)
{
	// UV値の中心からの距離(大きさ)
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
	// uv座標を-1 ~ +1に変換
	float2 pos = (uv - 0.5f) * 2.f;

	// 0 ~ 1の角度
	float angle = ((atan2(pos.r, pos.g) / 3.141592653589793f) + 1.f) / 2.f;

	// 角度の値を段階的にする
	static float divNum = 1024;
	float floorAngle = floor(angle * divNum) / divNum * seed;

	// (段階的な)角度を参考にノイズを返す
	return saturate(fracNoise(float2(floorAngle, floorAngle))) * colourIntensity;
}

float4 main(VSOutput input) : SV_TARGET
{
	static float PI = 3.141592653589793f;
	static float PI2 = 6.283185307179586f;
	
	// --------------------
	// モザイク
	// --------------------
	
	// 画面を左右それぞれmosaicNum分割した大きさのモザイクになる
	float2 uv = floor(input.uv * mosaicNum) / mosaicNum;
	// 時間[s]
	float time = nowTime / oneSec;

	// --------------------
	// ビネッタ
	// --------------------
	float vignNum = vignatte(uv);

	// --------------------
	// 走査線のようなもの
	// --------------------
	static float slnSpeed = 8.f;
	static float slnDivLevel = 96.f;
	static float slnPower = 24.f;
	float sinNum = uv.y * slnDivLevel + time * slnSpeed;
	float sLineNum = fracNoise(float2(time, uv.y)) * sin(sinNum) * sin(sinNum + 0.75f) + 1.f;
	sLineNum /= -slnPower;

	// --------------------
	// 走査線
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
	// rgbずらし
	// --------------------
	float4 texColor0 = tex0.Sample(smp, uv);
	texColor0.g = tex0.Sample(smp, uv + rgbShiftNum).g;
	static float gamma = 1.f / 2.2f;
	texColor0 = pow(texColor0, gamma);

	float noiseNum = noise(input.uv, time);
	
	// --------------------
	// 集中線
	// --------------------
	
	// 時間で動いてほしいのでseedには時間を入れる
	float speedLineNum = speedLine(input.uv, fmod(nowTime / oneSec, 1.f), speedLineIntensity);
	// 中心に近いほど色を薄くする
	speedLineNum *= distance(float2(0.5f, 0.5f), input.uv);

	float4 drawCol = float4(texColor0.rgb + sLineNum + vignNum + noiseNum + speedLineNum, alpha);

	return drawCol;
}
