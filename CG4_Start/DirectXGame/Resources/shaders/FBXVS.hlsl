#include "FBX.hlsli"

VSOutput main(VSInput input) {
	//ワールド行列によるスケーリング、回転を適用
	float4 wnormal = normalize(mul(matWorld , float4(input.normal , 0)));
	//ピクセルシェーダーに渡す値
	VSOutput output;
	//行列による座標変換
	output.svpos = mul(mul(viewProjection , matWorld) , input.pos);
	//ワールド法線を次のステージに渡す
	output.normal = wnormal.xyz;
	//入力値をそのまま次のステージに渡す
	output.uv = input.uv;

	return output;
}