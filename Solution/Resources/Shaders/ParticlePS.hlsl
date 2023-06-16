#include "Particle.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

PSOutput main(GSOutput input)
{
	PSOutput output;
	output.target0 = tex.Sample(smp, input.uv) * float4(input.color, 1.f);
	output.target1 = output.target0;

	return output;
}