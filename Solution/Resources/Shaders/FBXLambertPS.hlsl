#include "FBX.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

PSOutput main(VSOutput input)
{
	float4 texcolor = tex.Sample(smp, input.uv);
	
	PSOutput output;

	// Lambert反射
	float3 light = normalize(input.worldPos.xyz - lightPos);
	float diffuse = saturate(dot(-light, input.normal));
	float brightness = diffuse + 0.3f;
	float4 shadecolor = float4(brightness, brightness, brightness, 1.f);

	output.target0 = shadecolor * texcolor;
	output.target1 = output.target0;
	
	return output;
}