#define PI 3.14159
struct InstancedInformation
{
	float4x4 model;
	float4x4 normal;
	float roughness;
	float metallic;
	float3 albedo;
};
cbuffer SpotLightData : register(b0)
{
	float4x4 lightview[6];
	float4x4 lightproj;
	float4 lightpostion;
	float4 lightcolor;
	float4 lightattenuation;
	float lightradius;
	float far;
};
StructuredBuffer<InstancedInformation> instances: register(t0);
cbuffer RootConstant : register(b1)
{
	uint face;
};
struct PSInput
{
	float4 position : SV_POSITION;
};
PSInput VSMain(float3 position : POSITION, uint instanceid : SV_InstanceID)
{
	PSInput result;

	result.position = mul(instances[instanceid].model, float4(position, 1.0f));
	result.position = mul(lightview[face], result.position);
	result.position = mul(lightproj,result.position);
	return result;
}
