cbuffer cbuff0 : register(b0) {
	matrix viewProjection;
	matrix matWorld;
	float3 camerapos;
}

static const int MAX_BONES = 32;

cbuffer skinning:register(b3) {
	matrix matSkinning[MAX_BONES];
}

struct VSInput {
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	uint4 boneIndices : BONEINDICES;
	float4 boneWeights : BONEWEIGHTS;
};

struct VSOutput {
	float4 svpos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};