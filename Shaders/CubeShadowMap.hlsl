#include "GraphicsUtility.hlsl"
#define PI 3.14159
struct InstancedInformation
{
    Matrices matrices;
    Material material;
};


cbuffer SpotLightData : register(b0)
{
	float4x4 lightview[6];
	float4x4 lightproj;
	float4 lightpostion;
	float4 lightcolor;
	float lightradius;
    float lightintensity;
	float2 padding;
};
StructuredBuffer<InstancedInformation> instances: register(t0);
cbuffer RootConstant : register(b1)
{
	uint face;
};
struct PSInput
{
	float4 position : SV_POSITION;
    float3 wposition : W_POSITION;
};
struct GSOutput
{
    float4 position : SV_POSITION;
    float3 wposition : W_POSITION;
    uint face : SV_RenderTargetArrayIndex;
};
PSInput VSMain(float3 position : POSITION, uint instanceid : SV_InstanceID)
{
    PSInput result;

    result.position = mul(instances[instanceid].matrices.model, float4(position, 1.0f));
    result.wposition = result.position.xyz;
    result.position = mul(lightview[face], result.position);
    result.position = mul(lightproj, result.position);
    return result;
}
[maxvertexcount(3)]
void GSMain(triangle PSInput input[3], inout TriangleStream<GSOutput> stream)
{
    GSOutput output;
    output.face = face;
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        output.position = input[i].position;
        output.wposition = input[i].wposition;
        stream.Append(output);
    }

}

float PSMain(GSOutput input) : SV_Depth
{
    return length(input.wposition - lightpostion.xyz)/lightradius ;
}