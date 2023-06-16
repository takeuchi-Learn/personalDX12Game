cbuffer cbuff0 : register(b0)
{
	matrix mat; // ビュープロジェクション行列
	matrix matBillboard; // ビルボード行列
};

// 頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct VSOutput
{
	float4 pos : POSITION; // 頂点座標
	float scale : TEXCOORD; // スケール
	float3 color : COLOR;
};

struct GSOutput
{
	float4 svpos : SV_POSITION; // システム用頂点座標
	float2 uv : TEXCOORD; // uv値
	float3 color : COLOR;
};

// レンダーターゲットの数は2つ
struct PSOutput
{
	float4 target0 : SV_TARGET0;
	float4 target1 : SV_TARGET1;
};