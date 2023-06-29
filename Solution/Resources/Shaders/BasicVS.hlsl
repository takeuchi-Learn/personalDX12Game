#include "Basic.hlsli"

VSOutput main(float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, uint instNo : SV_InstanceID)
{
	float4 wnormal = normalize(mul(matWorld[instNo], float4(normal, 0)));
	float4 wpos = mul(matWorld[instNo], pos);
	
	VSOutput output;
	output.svpos = mul(mul(viewProj, matWorld[instNo]), pos); // 座標に行列を乗算
	output.worldPos = wpos;
	output.normal = wnormal.xyz;
	output.uv = uv;
	output.instNo = instNo;
	return output;
}
