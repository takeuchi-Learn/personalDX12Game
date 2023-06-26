#include "Basic.hlsli"

VSOutput main(float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, uint instNo : SV_InstanceID)
{
	float4 nowPos = pos;
	nowPos.x += pow(instNo, 2) * instCount / 10.f;
	
	float4 wnormal = normalize(mul(world, float4(normal, 0)));
	float4 wpos = mul(world, nowPos);
	
	VSOutput output;
	output.svpos = mul(mul(viewProj, world), nowPos); // 座標に行列を乗算
	output.worldPos = wpos;
	output.normal = wnormal.xyz;
	output.uv = uv;
	return output;
}
