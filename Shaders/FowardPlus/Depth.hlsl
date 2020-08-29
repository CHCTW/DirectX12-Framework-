#include "GraphicsUtility/GraphicsUtility.hlsl"
#include "GraphicsUtility/LightFunctions.hlsl"
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
cbuffer Objecetid : register(b1)
{
    uint ojectindex;
};

StructuredBuffer<Object> ObjectList : register(t0);
StructuredBuffer<Matrices> MatricesList : register(t1);
StructuredBuffer<Material> MaterialList : register(t2);
Texture2D MaterialTextures[] : register(t0, space1);
SamplerState mat_text_sampler : register(s0);
struct PSInput
{
	float4 position : SV_POSITION;
    float2 uv : TEXTCOORD;
    nointerpolation Material mat : MATERIAL;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL,float2 uv : TEXTURECOORD, float3 tangent: TANGENT,float3 bitangent : BITANGENT)
{
	PSInput result;
    uint mid = ObjectList[ojectindex].matricesid;
    result.position = mul(MatricesList[mid].model, float4(position, 1.0f));
    result.position = mul(camera.view, result.position);

    result.position = mul(camera.proj, result.position);
    result.uv = uv;
    result.mat = MaterialList[ObjectList[ojectindex].materialid];
	return result;
}
void PSMain(PSInput input)
{
    uint diffid = input.mat.texutres.x;
    float rate = input.mat.chooses.x;
    float4 difftext = MaterialTextures[diffid].Sample(mat_text_sampler, input.uv).rgba;
    if (difftext.a == 0)
        discard;
}