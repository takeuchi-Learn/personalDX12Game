#include "Basic.hlsli"

VSOutput main(float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	//VSOutput output; // ピクセルシェーダーに渡す値
	//output.svpos = mul(mat, pos); // 座標に行列を乗算
	//output.normal = normal;
	//output.uv = uv;
	//return output;

	/*float3 normalLight = normalize(light);

	float3 lightColor = float3(1, 1, 1);*/

    float4 wnormal = normalize(mul(world, float4(normal, 0)));
    float4 wpos = mul(world, pos);

	

    VSOutput output;
    output.svpos = mul(mul(viewProj, world), pos); // 座標に行列を乗算
    output.worldPos = wpos;
    output.normal = wnormal.xyz;
    output.uv = uv;
    return output;
}
