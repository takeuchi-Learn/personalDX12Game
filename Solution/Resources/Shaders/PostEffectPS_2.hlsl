#include "PostEffect.hlsli"

Texture2D<float4> tex0 : register(t0);   // 0番スロットに設定されたテクスチャ
Texture2D<float4> tex1 : register(t1);   // 1番スロットに設定されたテクスチャ
SamplerState smp : register(s0);         // 0番スロットに設定されたサンプラー

float4 main(VSOutput input) : SV_TARGET
{
	float4 texColor0 = tex0.Sample(smp, input.uv);
	float4 texColor1 = tex1.Sample(smp, input.uv);

	// tex0とtex1の横縞になるように描画
	float4 col = lerp(texColor0, texColor1, step(0.05f, fmod(input.uv.y, 0.1f)));

	return col;
}
