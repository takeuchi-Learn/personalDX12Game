#include "Basic.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

PSOutput main(VSOutput input)
{
	clip(input.instNo < instCount ? 1 : -1);

	float4 texcolor = tex.Sample(smp, input.uv * texTilling + shiftUv);
	
	PSOutput output;
	output.target0 = texcolor * color[input.instNo];
	output.target1 = output.target0;

	return output;
}
