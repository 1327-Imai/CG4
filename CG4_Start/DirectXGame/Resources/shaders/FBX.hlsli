cbuffer cbuff0 : register(b0) {
	matrix viewProjection;
	matrix matWorld;
	float3 camerapos;
}

struct VSInput {
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VSOutput {
	float4 svpos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};