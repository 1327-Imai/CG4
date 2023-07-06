#include "PostEffectTest.hlsli"

Texture2D<float4> tex : register(t0);  //0番スロットに設定されたテクスチャ
SamplerState smp : register(s0);      //0番スロットに設定されたサンプラー

float4 main(VSOutput input) : SV_TARGET
{
	//通常
	float4 texcolor = tex.Sample(smp , input.uv);
	//Uvずらし
	//float4 texcolor = tex.Sample(smp , input.uv + float2(0.5,0));

	////平均ぼかし
	//float shiftPixelU = 1.0f / 1280.0f;
	//float shiftPixelV = 1.0f / 720.0f;
	//
	//float4 color = {0 , 0 , 0 , 0};
	//int shiftValueX = 5;
	//int shiftValueY = 5;
	//
	//for (int i = 0; i < shiftValueX; i++) {
	//	for (int j = 0; j < shiftValueY; j++) {
	//
	//		color += tex.Sample(smp , input.uv + float2(-shiftPixelU *j , -shiftPixelV * i));
	//		color += tex.Sample(smp , input.uv + float2(0 , -shiftPixelV * i));
	//		color += tex.Sample(smp , input.uv + float2(shiftPixelU * j , -shiftPixelV * i));
	//
	//		color += tex.Sample(smp , input.uv + float2(-shiftPixelU * j , 0));
	//		color += tex.Sample(smp , input.uv + float2(0 , 0));
	//		color += tex.Sample(smp , input.uv + float2(shiftPixelU * j , 0));
	//
	//		color += tex.Sample(smp , input.uv + float2(-shiftPixelU * j , shiftPixelV * i));
	//		color += tex.Sample(smp , input.uv + float2(0 , shiftPixelV * i));
	//		color += tex.Sample(smp , input.uv + float2(shiftPixelU * j , shiftPixelV * i));
	//	}
	//}

	//color.rgb /= shiftValueX * shiftValueY * 9;
	//color.a = 1;
	//
	//return color;

	//通常
	return float4(texcolor.rgb , 1);
	//明度2倍
	//return float4(texcolor.rgb * 2.0f, 1);
	//色反転
	//return float4(1 - texcolor.rgb , 1);
	
}