#include "Basic.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

#define ditherLevelMax (16.f)

// 光沢
#define shininess (4.f)

// ディザリング抜き
void ScreenDoor(float2 screenPos, float alpha)
{
	static const int Bayer[4][4] =
	{
		{ 0, 8, 2, 10 },
		{ 12, 4, 14, 6 },
		{ 3, 11, 1, 9 },
		{ 15, 7, 13, 5 }
	};
	
	// 0 ~ ditherLevelMax
	float ditherLevel = clamp(ditherLevelMax - (alpha * ditherLevelMax), 0.f, ditherLevelMax);
		
	int2 ditherUv = int2((int)fmod(screenPos.x, 4.f), (int)fmod(screenPos.y, 4.f));
	float doorNum = Bayer[ditherUv.y][ditherUv.x];
	clip(doorNum - ditherLevel);
}

PSOutput main(VSOutput input)
{
	clip(input.instNo < instCount ? 1 : -1);
	
	float4 texcolor = color[input.instNo] * float4(tex.Sample(smp, input.uv * texTilling + shiftUv));

	ScreenDoor(input.svpos.xy, texcolor.a * m_alpha);

	float3 eyeDir = normalize(cameraPos - input.worldPos.xyz); // 頂点->視点ベクトル

	float3 dir2Light = normalize(lightPos - input.worldPos.xyz);

	float3 dir2LightDotNormal = dot(dir2Light, input.normal);

	float3 reflect = normalize(-dir2Light + 2 * dir2LightDotNormal * input.normal); // 反射光

	float3 ambient = m_ambient;
	float3 diffuse = dir2LightDotNormal * m_diffuse;
	float3 specular = pow(saturate(dot(reflect, eyeDir)), shininess) * m_specular; // 鏡面反射光

	float4 shadeColor;
	shadeColor.rgb = (ambient + diffuse + specular) * lightColor.rgb;
	shadeColor.a = m_alpha;
	
	PSOutput output;
	output.target0 = shadeColor * texcolor;
	// target1を反転色にする
	output.target1 = output.target0;

	return output;
}
