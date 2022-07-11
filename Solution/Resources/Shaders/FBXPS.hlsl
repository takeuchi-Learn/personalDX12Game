#include "FBX.hlsli"

Texture2D<float4> tex : register(t0);  	// 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp : register(s0);      	// 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

PSOutput main(VSOutput input) {
	PSOutput output;

	float3 eyeDir = normalize(cameraPos - input.worldPos.xyz);   // ���_->���_�x�N�g��

	const float shininess = 4.f;    // ����

	float3 dir2Light = normalize(lightPos - input.worldPos.xyz);

	float3 dir2LightDotNormal = dot(dir2Light, input.normal);

	float3 reflect = normalize(-dir2Light + 2 * dir2LightDotNormal * input.normal); // ���ˌ�

	float3 ambient = m_ambient;
	float3 diffuse = dir2LightDotNormal * m_diffuse;
	float3 specular = pow(saturate(dot(reflect, eyeDir)), shininess) * m_specular;  // ���ʔ��ˌ�

	float4 shadeColor;
	shadeColor.rgb = (ambient + diffuse + specular) * lightColor;
	shadeColor.a = m_alpha;

	float4 texcolor = float4(tex.Sample(smp, input.uv));
	output.target0 = shadeColor * texcolor;
	// target1�𔽓]�F�ɂ���
	output.target1 = output.target0;

	return output;
}