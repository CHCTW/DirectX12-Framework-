#include "GraphicsUtility/GraphicsUtility.hlsl"
#define PI 3.14159
struct InstancedInformation
{
    Matrices matrices;
    Material material;
};


cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float3 eye;
	float padding;
};



StructuredBuffer<InstancedInformation> instances: register(t0);

struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 pos: POS;
	nointerpolation uint id : ID;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, uint instanceid : SV_InstanceID)
{
	PSInput result;

	result.position = mul(instances[instanceid].matrices.model, float4(position, 1.0f));
	result.pos = result.position.xyz;
	result.position = mul(view, result.position);
	result.position = mul(proj,result.position);
	
    result.normal = mul(instances[instanceid].matrices.normal, float4(normal, 0.0f)).xyz;
	result.id = instanceid;
	return result;
}
struct PSOut
{
	float4 NorMet : COLOR0;
	float4 AlbRou : COLOR1;
};

PSOut PSMain(PSInput input) : SV_TARGET
{
	PSOut res;
	float roughness = instances[input.id].material.roughness;
    float metallic = instances[input.id].material.metallic;
    float3 albedo = instances[input.id].material.albedo;
	float ao = 1.0f;
	res.NorMet.xyz = input.normal;

//	res.NorMet.xyz = input.pos;
	res.NorMet.w = metallic;

	res.AlbRou.xyz = albedo;
	res.AlbRou.w = roughness;
	return res;
}
