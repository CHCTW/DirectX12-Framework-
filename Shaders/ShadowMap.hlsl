#include "GraphicsUtility/GraphicsUtility.hlsl"
#define PI 3.14159
struct InstancedInformation
{
    Matrices matrices;
    Material material;
};
cbuffer SpotLightData : register(b0)
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
};
PSInput VSMain(float3 position : POSITION, uint instanceid : SV_InstanceID)
{
	PSInput result;

    result.position = mul(instances[instanceid].matrices.model, float4(position, 1.0f));
	result.position = mul(lightview, result.position);
	result.position = mul(lightproj,result.position);
 
    result.position /= result.position.w;
	return result;
}
