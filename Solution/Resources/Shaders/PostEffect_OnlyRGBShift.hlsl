#include "PostEffect.hlsli"

Texture2D<float4> tex0 : register(t0); // 0番スロットに設定されたテクスチャ
Texture2D<float4> tex1 : register(t1); // 1番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

float4 main(VSOutput input) : SV_TARGET
{

	// rgbずらし
	float4 texColor0;
	
	texColor0.g = tex0.Sample(smp, input.uv + rgbShiftNum).g;
	texColor0.rba = tex0.Sample(smp, input.uv).gba;

	float4 drawCol = float4(texColor0.rgb, alpha);

	return drawCol;
}
