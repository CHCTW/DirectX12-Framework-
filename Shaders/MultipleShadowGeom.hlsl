#include "GraphicsUtility.hlsl"
#include "LightFunctions.hlsl"
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
    float3 wposition : WPOSITION;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXTCOORD;
    nointerpolation Material mat : MATERIAL;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL,float2 uv : TEXTURECOORD, float3 tangent: TANGENT)
{
	PSInput result;
    uint mid = ObjectList[ojectindex].matricesid;
    result.position = mul(MatricesList[mid].model, float4(position, 1.0f));
    result.wposition = mul(MatricesList[mid].model, float4(position, 1.0f)).xyz;
    result.position = mul(camera.view, result.position);
    result.position = mul(camera.proj, result.position);
    result.normal = mul(MatricesList[mid].normal, float4(normal, 0.0)).xyz;
    result.tangent = mul(MatricesList[mid].normal, float4(tangent, 0.0)).xyz;
    result.uv = uv;
    result.mat = MaterialList[ObjectList[ojectindex].materialid];
   // result.normal = normal;
	return result;
}

GBufferFormat PSMain(PSInput input) : SV_TARGET
{
    GBufferFormat res;
    uint diffid = input.mat.texutres.x;
    float rate = input.mat.chooses.x;
    float4 difftext = MaterialTextures[diffid].Sample(mat_text_sampler, input.uv).rgba;
    if(difftext.a==0)
        discard;
    float3 diffuse = difftext.xyz * (1.0 - rate) + input.mat.albedo * rate;


    float3 normaltext = MaterialTextures[input.mat.texutres.y].Sample(mat_text_sampler, input.uv) * 2.0 - 1.0;
    float normalrate = input.mat.chooses.y;
    float3 normal = normalMapCal(normalrate,input.normal, input.tangent, normaltext);


    float roughrate  = input.mat.chooses.z;
    float rough = MaterialTextures[input.mat.texutres.z].Sample(mat_text_sampler, input.uv).r * (1 - roughrate) + roughrate * input.mat.roughness;

    float metalicrate = input.mat.chooses.w;
    float metalic = MaterialTextures[input.mat.texutres.w].Sample(mat_text_sampler, input.uv).r * (1 - roughrate) + roughrate * input.mat.metallic;

    half2 en = encode(normal);
    float3 decodenormal = decode(en);


    /****************************************/
 //   float3 albedo = pow(diffuse, 2.2);
 //   float ao = 1.0f;


 //   float3 F0 = float3(0.04, 0.04, 0.04);
 //   F0 = lerp(F0, albedo, float3(metalic, metalic, metalic)); // use metalic value to get F



 //   float3 N = normalize(decodenormal);
 //   float3 V = normalize(mPosition.xyz - input.wposition);
 //   float3 L = normalize(float3(0, 20, 0) - input.wposition);
 //   float3 H = normalize(L + V);
 //   float LH = max(dot(L, H), 0.0f);
 //   float NL = max(dot(L, N), 0.0f);
 //   float NV = max(dot(N, V), 0.0f);
 //   float HV = max(dot(H, V), 0.0f);

 //   float3 F = Fresnel(F0, HV);
 //   float D = Distribution(N, H, rough);
 //   float3 G = GeometrySmith(NV, NL, rough);
 //  // return float4(F.y, F.y, F.y,1.0);


 //   float3 spec = D * F * G / (4 * NV * NL + 0.001);


 //   float3 Kd = 1 - F; // diffuse color 
 //   Kd = Kd * (1.0 - metalic);
 //   float3 diff = Kd * albedo / PI;




 //   float dist = length(float3(0, 20, 0) - input.wposition);
	////float att = 1.0f / (1 + lightattenuation.y*dist + dist*dist*lightattenuation.z);

 //   //float3 plightspace = normalize(mul(lightview, float4(input.wposition.xyz, 1.0))); // get diredtion from light point to poision in light spcace;
 //  // return float4(plightspace, 1.0);

 //   //float angle = dot(plightspace, float3(0, 0, -1));
 // //  return float4(angle, angle, angle, angle);
    

 // //  float ta = saturate(angle - lightconeangle) / (1.0 - lightconeangle);


 //   float t = pow(dist / 400, 4);
 //   float att = pow(saturate(1 - t), 2) / (dist * dist + 1);


 //   float3 final = (diff + spec) * NL * 3000 * att * float3(1.0, 0.3, 0.0) + albedo * 0.05;
 //   final = final / (1 + final); // tone mapping
 //   final = pow(final, 1.0f / 2.2f);
 //   return float4(final, 1.0f);


    res.normal = en;
    res.albedorough = float4(diffuse, rough);
    res.emmermetal.w = metalic;

    return res;


 //   return float4(normal, 1.0);

}
