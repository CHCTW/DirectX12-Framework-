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
StructuredBuffer<uint> PointLightIndexList : register(t3);
StructuredBuffer<uint> BlockLightCount : register(t4);
StructuredBuffer<SceneData> sceneData : register(t5);
StructuredBuffer<PointLightData> PointLightDataList : register(t6);

struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 wPosition : POSITION;
    float2 uv : TEXTCOORD;
    nointerpolation Material mat : MATERIAL;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL,float2 uv : TEXTURECOORD, float3 tangent: TANGENT,float3 bitangent : BITANGENT)
{
	PSInput result;
    uint mid = ObjectList[ojectindex].matricesid;
    result.position = mul(MatricesList[mid].model, float4(position, 1.0f));
    result.wPosition = result.position.xyz;
    result.position = mul(camera.view, result.position);

    result.position = mul(camera.proj, result.position);
    result.normal = mul(MatricesList[mid].normal, float4(normal, 0.0)).xyz;
    result.tangent = mul(MatricesList[mid].normal, float4(tangent, 0.0)).xyz;
    result.bitangent = mul(MatricesList[mid].normal, float4(bitangent, 0.0)).xyz;
    result.uv = uv;
    result.mat = MaterialList[ObjectList[ojectindex].materialid];
   // result.normal = normal;
	return result;
}
float4 PSMain(PSInput input) : SV_TARGET
{
    
    uint2 id = input.position.xy;
    uint2 groupId = uint2(id.x / BLOCKSIZE, id.y / BLOCKSIZE);
    uint widthBlockNumber = (sceneData[0].frameWidth + BLOCKSIZE - 1) / BLOCKSIZE;
    uint globalIndex = groupId.y * widthBlockNumber + groupId.x;
   
    uint lightCount = BlockLightCount[globalIndex*2];
    //uint offset = globalIndex * sceneData[0].pointLightNumber;
    uint offset = BlockLightCount[globalIndex * 2 + 1];
    uint diffid = input.mat.texutres.x;
    float rate = input.mat.chooses.x;
    float4 difftext = MaterialTextures[diffid].Sample(mat_text_sampler, input.uv).rgba;
    if(difftext.a==0.0)
        discard;
    float3 diffuse = difftext.xyz * (1.0 - rate) + input.mat.albedo * rate;
    float3 albedo = pow(diffuse.rgb, 2.2);

    float3 normaltext = (MaterialTextures[input.mat.texutres.y].Sample(mat_text_sampler, input.uv) * 2.0 - 1.0).xyz;
    float normalrate = input.mat.chooses.y;
    float3 normal =normalMapCal(normalrate,input.normal,input.tangent, input.bitangent, normaltext);
    //normal = normalize(input.normal);

    float roughrate  = input.mat.chooses.z;
    float rough = MaterialTextures[input.mat.texutres.z].Sample(mat_text_sampler, input.uv).r * (1 - roughrate) + roughrate * input.mat.roughness;

    float metalicrate = input.mat.chooses.w;
    float metalic = MaterialTextures[input.mat.texutres.w].Sample(mat_text_sampler, input.uv).r * (1 - roughrate) + roughrate * input.mat.metallic;
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, float3(metalic, metalic, metalic)); // use metalic value to get F

    float3 N = normal;
    float3 V = normalize(camera.eye-input.wPosition);
    float3 total = float3(0.0,0.0,0.0);
    
    
    float c = 0.0f;
    for (uint i = 0; i < lightCount; ++i)
    {
        uint lightId = PointLightIndexList[offset + i+1];
        float3 l = PointLightDataList[lightId].
        lightposition.xyz - input.wPosition;
        float dist = length(l);
        if (dist > PointLightDataList[lightId].lightradius)
            continue;
        c += 1.0f;
        float3 L = l/dist;
        float3 H = normalize(L + V);
        float LH = max(dot(L, H), 0.0f);
        float NL = max(dot(L, N), 0.0f);
        float NV = max(dot(N, V), 0.0f);
        float HV = max(dot(H, V), 0.0f);
        float3 F = Fresnel(F0, HV);
        float D = Distribution(N, H, rough);
        float3 G = GeometrySmith(NV, NL, rough);
        float3 spec = D * F * G / (4 * NV * NL + 0.001);
        
        float3 Kd = 1 - F; // diffuse color 
        Kd = Kd * (1.0 - metalic);
        float3 diff = Kd * albedo / PI;
        
        float t = pow(dist / PointLightDataList[lightId].lightradius, 4);
        float att = pow(saturate(1 - t), 2) / (dist * dist + 1);
        total += (diff + spec) * NL * PointLightDataList[lightId].lightcolor.xyz * att * PointLightDataList[lightId].lightintensity;

    }
    c = lightCount;
    c /= sceneData[0].pointLightNumber;
    //c *= 4.0;
    if (sceneData[0].debugDraw)
        return float4(0, c, c, 1.0);
    return float4(total, 0.5);

}
