cbuffer cbuff0 : register(b0)
{
	float4 color; // 色(RGBA)
	//matrix mat; // ３Ｄ変換行列
	matrix viewProj;
	matrix world; // ワールド行列
	float3 cameraPos; // カメラ位置(ワールド座標)
};

cbuffer cbuff1 : register(b1)
{
	float3 m_ambient : packoffset(c0); // アンビエント係数
	float3 m_diffuse : packoffset(c1); // ディフューズ係数
	float3 m_specular : packoffset(c2); // ディフューズ係数
	float m_alpha : packoffset(c2.w); // アルファ
	float2 texTilling : packoffset(c3);
	float2 shiftUv : packoffset(c4);
}

cbuffer cbuff2 : register(b2)
{
	float3 lightPos; // ライトの位置(ワールド)
	float3 lightColor; // ライトの色(RGB)
};

// 頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct VSOutput
{
	float4 svpos : SV_POSITION; // システム用頂点座標
	float4 worldPos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD; // uv値
};

// レンダーターゲットの数は2つ
// PostEffectクラスのRenderTargetNumは此処の個数(2つ)に合わせる
// 配列にすればいけるのでは?
struct PSOutput
{
	float4 target0 : SV_TARGET0;
	float4 target1 : SV_TARGET1;
};