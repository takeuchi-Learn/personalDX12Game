cbuffer cbuff0 : register(b0)
{
	float oneSec; // 一秒(時間の単位確認用)
	float nowTime; // 現在の時間
	float2 winSize; // 画面サイズ
	float noizeIntensity; // ノイズ係数
	float2 mosaicNum; // モザイクの分割数
	float vignIntensity; // ビネッタ係数
	float alpha; // 不透明度(通常は1)
	float2 rgbShiftNum;
	float speedLineIntensity;
};

// 頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct VSOutput
{
	float4 svpos : SV_POSITION; // システム用頂点座標
	float2 uv : TEXCOORD; // uv値
};
