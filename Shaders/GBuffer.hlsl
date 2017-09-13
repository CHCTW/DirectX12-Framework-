#define PI 3.14159
struct InstancedInformation
{
	float4x4 model;
	float4x4 normal;
	float roughness;
	float metallic;
	float3 albedo;
};

cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float3 eye;
	float padding;
	float4x4 projinv;
};


cbuffer SpotLightData : register(b1)
{
	float4x4 lightview;
	float4x4 lightproj;
	float4 lightpostion;
	float4 lightcolor;
	float4 lightattenuation;
	float lightradius;
};
StructuredBuffer<InstancedInformation> instances: register(t0);

struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	uint id : ID;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, uint instanceid : SV_InstanceID)
{
	PSInput result;

	result.position = mul(instances[instanceid].model, float4(position, 1.0f));
	result.position = mul(view, result.position);
	result.position = mul(proj,result.position);
	
	result.normal = mul(instances[instanceid].normal, float4(normal, 0.0f)).xyz;
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
	float roughness = instances[input.id].roughness;
	float metallic = instances[input.id].metallic;
	float3 albedo = instances[input.id].albedo;
	float ao = 1.0f;
	res.NorMet.xyz = input.normal;
	res.NorMet.w = metallic;

	res.AlbRou.xyz = albedo;
	res.AlbRou.w = roughness;
	return res;
}
