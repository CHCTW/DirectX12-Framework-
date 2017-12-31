#include "../GraphicsUtility/GraphicsUtility.hlsl"
#include "../GraphicsUtility/LightFunctions.hlsl"
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
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
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
    result.normal = mul(MatricesList[mid].normal, float4(normal, 0.0)).xyz;
    result.normal =  mul(camera.viewinverseyranspose, float4(result.normal, 0.0)).xyz;
    result.tangent = mul(MatricesList[mid].normal, float4(tangent, 0.0)).xyz;
    result.tangent = mul(camera.viewinverseyranspose, float4(result.tangent, 0.0)).xyz;

    result.bitangent = mul(MatricesList[mid].normal, float4(bitangent, 0.0)).xyz;
    result.bitangent = mul(camera.viewinverseyranspose, float4(result.bitangent, 0.0)).xyz;


    result.uv = uv;
    result.mat = MaterialList[ObjectList[ojectindex].materialid];
   // result.normal = normal;
	return result;
}
//[maxvertexcount(3)]
//void GSMain(triangle PSInput gin[3], inout TriangleStream<PSInput> stream)
//{
//    float3 vec1 = gin[1].vposition.xyz - gin[0].vposition.xyz;
//    float3 vec2 = gin[2].vposition.xyz - gin[0].vposition.xyz;
//    float3 flatn = cross(vec1, vec2);
//    if(dot(-gin[1].vposition.xyz,flatn)>0)
//    {
//        stream.Append(gin[0]);
//        stream.Append(gin[1]);
//        stream.Append(gin[2]);

//    }

//}
    GBufferFormat PSMain(PSInput input)
{
    GBufferFormat res;
    uint diffid = input.mat.texutres.x;
    float rate = input.mat.chooses.x;
    float4 difftext = MaterialTextures[diffid].Sample(mat_text_sampler, input.uv).rgba;
    if(difftext.a==0)
        discard;
    float3 diffuse = difftext.xyz * (1.0 - rate) + input.mat.albedo * rate;


    float3 normaltext = (MaterialTextures[input.mat.texutres.y].Sample(mat_text_sampler, input.uv) * 2.0 - 1.0).xyz;
    float normalrate = input.mat.chooses.y;
    float3 normal =normalMapCal(normalrate,input.normal,input.tangent, input.bitangent, normaltext);
    //normal = normalize(input.normal);

    float roughrate  = input.mat.chooses.z;
    float rough = MaterialTextures[input.mat.texutres.z].Sample(mat_text_sampler, input.uv).r * (1 - roughrate) + roughrate * input.mat.roughness;

    float metalicrate = input.mat.chooses.w;
    float metalic = MaterialTextures[input.mat.texutres.w].Sample(mat_text_sampler, input.uv).r * (1 - roughrate) + roughrate * input.mat.metallic;

    half2 en = encode(normal);
    res.normal = en;

    res.albedorough = float4(diffuse, rough);
    res.emmermetal.w = metalic;
    res.emmermetal.xyz = metalic.xxx;
    return res;


 //   return float4(normal, 1.0);

}
