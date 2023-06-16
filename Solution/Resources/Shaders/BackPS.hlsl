#include "Back.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

PSOutput main(VSOutput input)
{
	PSOutput output;

	float4 texcolor = float4(tex.Sample(smp, input.uv * texTilling + shiftUv));
	output.target0 = texcolor * color;
	output.target1 = output.target0;

	return output;
}
