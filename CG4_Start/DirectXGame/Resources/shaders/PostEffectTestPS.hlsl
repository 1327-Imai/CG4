#include "PostEffectTest.hlsli"

Texture2D<float4> tex0 : register(t0);  //0番スロットに設定されたテクスチャ
Texture2D<float4> tex1 : register(t1);  //1番スロットに設定されたテクスチャ
SamplerState smp : register(s0);      //0番スロットに設定されたサンプラー

////通常
//float4 main(VSOutput input) : SV_TARGET
//{
//	float4 texcolor = tex.Sample(smp , input.uv);
//	return float4(texcolor.rgb , 1);
//}

////UVずらし
//float4 main(VSOutput input) : SV_TARGET
//{
//	float4 texcolor = tex.Sample(smp , input.uv + float2(0.5,0));
//	return float4(texcolor.rgb , 1);
//}

//float4 main(VSOutput input) : SV_TARGET
//{
//	//平均ぼかし
//	float shiftPixelU = 1.0f / 1280.0f;
//	float shiftPixelV = 1.0f / 720.0f;
//	
//	float4 color = {0 , 0 , 0 , 0};
//	int shiftValueX = 5;
//	int shiftValueY = 5;
//	
//	for (int i = 0; i < shiftValueX; i++) {
//		for (int j = 0; j < shiftValueY; j++) {
//	
//			color += tex.Sample(smp , input.uv + float2(-shiftPixelU *j , -shiftPixelV * i));
//			color += tex.Sample(smp , input.uv + float2(0 , -shiftPixelV * i));
//			color += tex.Sample(smp , input.uv + float2(shiftPixelU * j , -shiftPixelV * i));
//	
//			color += tex.Sample(smp , input.uv + float2(-shiftPixelU * j , 0));
//			color += tex.Sample(smp , input.uv + float2(0 , 0));
//			color += tex.Sample(smp , input.uv + float2(shiftPixelU * j , 0));
//	
//			color += tex.Sample(smp , input.uv + float2(-shiftPixelU * j , shiftPixelV * i));
//			color += tex.Sample(smp , input.uv + float2(0 , shiftPixelV * i));
//			color += tex.Sample(smp , input.uv + float2(shiftPixelU * j , shiftPixelV * i));
//		}
//	}
//
//	color.rgb /= shiftValueX * shiftValueY * 9;
//	color.a = 1;
//	
//	return color;
//}

////色反転
//float4 main(VSOutput input) : SV_TARGET
//{
//	float4 texcolor = tex.Sample(smp , input.uv);
//	return float4(1 - texcolor.rgb , 1);
//}

float4 main(VSOutput input) : SV_TARGET
{
	float4 colortex0 = tex0.Sample(smp , input.uv);
	float4 colortex1 = tex1.Sample(smp , input.uv);

	float4 color = colortex0;
	if (fmod(input.uv.y , 0.1f) < 0.05f) {
		color = colortex1;
	}

	return float4(color.rgb , 1);
}