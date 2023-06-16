#include "Basic.hlsli"

VSOutput main(float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	float4 wnormal = normalize(mul(world, float4(normal, 0)));
	float4 wpos = mul(world, pos);
	
	VSOutput output;
	output.svpos = mul(mul(viewProj, world), pos); // 座標に行列を乗算
	output.worldPos = wpos;
	output.normal = wnormal.xyz;
	output.uv = uv;
	return output;
}
