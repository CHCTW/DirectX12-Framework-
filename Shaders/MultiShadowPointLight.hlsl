#include "GraphicsUtility/Lightfunctions.hlsl"
#include "GraphicsUtility/GraphicsUtility.hlsl"
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
cbuffer PointLightid : register(b1)
{
    uint pointlightindex;
};
Texture2D GBufferTextures[4] : register(t0);
// decode normal
// albedo + rougbess
//  unused + metlaic
//  depth
StructuredBuffer<PointLightData> PointLightList : register(t4);
TextureCubeArray shadowmaps : register(t5);
SamplerState g_sampler : register(s0);
struct PSInput
{
	float4 position : SV_POSITION;
    nointerpolation float3 viewlightpositin : VIEWLIGHTPOSITION;
    nointerpolation float3 worldlightpositin : WORLDLIGHTPOSITION;
    nointerpolation uint id : ID;
};
struct GSInput
{
    nointerpolation uint id : ID;
};
GSInput VSMain(uint vid : SV_InstanceID)
{
    GSInput result;
 
	result.id = pointlightindex;
	return result;
}
[maxvertexcount(4)]
void GSMain(point GSInput gin[1], inout TriangleStream<PSInput> stream)
{
    PSInput res;
    res.id = gin[0].id;

    res.worldlightpositin = PointLightList[res.id].lightposition.xyz;
    float4 vspaceposition = mul(camera.view, PointLightList[res.id].lightposition);
    res.viewlightpositin = vspaceposition.xyz;
    

    float radius = PointLightList[res.id].lightradius;
    float def = 1.0;
    float3 left = def.xxx, right = def.xxx, top = def.xxx, bot = def.xxx;
    BoundsforAxis(float3(1, 0, 0), vspaceposition.xyz, radius, -camera.front, left, right);
    BoundsforAxis(float3(0, 1, 0), vspaceposition.xyz, radius, -camera.front, bot, top);


    float4 leftscreen = mul(camera.proj, float4(left, 1.0));
    float4 rightscreen = mul(camera.proj, float4(right, 1.0));
    float4 botscreen = mul(camera.proj, float4(bot, 1.0));
    float4 topscreen = mul(camera.proj, float4(top, 1.0));

    leftscreen.xy /= leftscreen.w;
    rightscreen.xy /= rightscreen.w;
    botscreen.xy /= botscreen.w;
    topscreen.xy /= topscreen.w;


    res.position.zw = float2(1.0, 1.0);

    // generate bound ss quad 
    res.position.x = leftscreen.x;
    res.position.y = topscreen.y;
    stream.Append(res);


    res.position.x = rightscreen.x;
    res.position.y = topscreen.y;
    stream.Append(res);


    res.position.x = leftscreen.x;
    res.position.y = botscreen.y;
    stream.Append(res);


    res.position.x = rightscreen.x;
    res.position.y = botscreen.y;
    stream.Append(res);


}
static const uint samplecount = 20;
static float3 pcfs[20] =
{
    float3(1, 1, 1), float3(1, -1, 1), float3(-1, -1, 1), float3(-1, 1, 1),
   float3(1, 1, -1), float3(1, -1, -1), float3(-1, -1, -1), float3(-1, 1, -1),
   float3(1, 1, 0), float3(1, -1, 0), float3(-1, -1, 0), float3(-1, 1, 0),
   float3(1, 0, 1), float3(-1, 0, 1), float3(1, 0, -1), float3(-1, 0, -1),
   float3(0, 1, 1), float3(0, -1, 1), float3(0, -1, -1), float3(0, 1, -1)
};

    float4 PSMain(PSInput input) : SV_TARGET
{
	//input.position.x/1920
    float2 uv = float2(input.position.x / 1600, input.position.y / 900);


    

    float3 normal = decode(GBufferTextures[0].Sample(g_sampler, uv).rg);
   // normal = GBufferTextures[0].Sample(g_sampler, uv).rgb;
  //  return float4(1.0, 1.0, 0.0, 1.0);
   // return float4(normal, 1.0);
    float metallic = GBufferTextures[2].Sample(g_sampler, uv).a;
    float3 albedo = pow(GBufferTextures[1].Sample(g_sampler, uv).rgb,2.2);
    float roughness = GBufferTextures[1].Sample(g_sampler, uv).a;
    float depth = GBufferTextures[3].Sample(g_sampler, uv).r;
    float4 projcoord;
       projcoord.xy = uv;
    projcoord.y = 1.0 - projcoord.y;
    projcoord.xy = projcoord.xy * 2.0f - 1.0f;
    projcoord.z = depth;
    projcoord.w = 1.0f;
    float4 pos = mul(camera.projinverse, projcoord);
    pos.xyzw /= pos.w;
    

    float4 wpos = mul(camera.viewinverse, pos);
    wpos.xyz = wpos.xyz / wpos.w;

//	return wpos;
    float dist = length(input.viewlightpositin - pos.xyz);
    if (dist > PointLightList[input.id].lightradius)
        discard;


    //return pos;
    float3 shadowcood = wpos.xyz-PointLightList[input.id].lightposition.xyz;
    float lightdepth = shadowmaps.Sample(g_sampler, float4(shadowcood, input.id)).r * PointLightList[input.id].lightradius;
    float pixdepth = length(shadowcood);
    float shadow = 0.0f;
    float diskradius = 0.1;
    float test = (pixdepth < lightdepth + 0.5f);



  ////  [unroll]
  //  for (uint i = 0; i < samplecount; ++i)
  //  {
  //      lightdepth = shadowmaps.SampleLevel(g_sampler, float4(shadowcood * 256 / pixdepth + pcfs[i], input.id), 0).r;
  //      lightdepth *= PointLightList[input.id].lightradius;
  //      if (pixdepth > (lightdepth + 0.1f))
  //          shadow += 1.0;


  //  }

  //  test = 1 - (shadow / (float) samplecount);
   
   

   //  return float4(albedo, 0.0);
  //  return float4(albedo, 0.0);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F

    float3 N = normal;
    float3 V = normalize(-pos.xyz);
    float3 L = normalize(input.viewlightpositin - pos.xyz);
    float3 H = normalize(L + V);
    float LH = max(dot(L, H), 0.0f);
    float NL = max(dot(L, N), 0.0f);
    float NV = max(dot(N, V), 0.0f);
    float HV = max(dot(H, V), 0.0f);

//	return float4(pos.xyz, 1.0);

    float3 F = Fresnel(F0, HV);
    float D = Distribution(N, H, roughness);
    float3 G = GeometrySmith(NV, NL, roughness);

    float3 spec = D * F * G / (4 * NV * NL + 0.001);


    float3 Kd = 1 - F; // diffuse color 
    Kd = Kd * (1.0 - metallic);
    float3 diff = Kd * albedo / PI;


 //   float dp = dist/(1 - dist / LocalLightData[input.id].lightradius * dist / LocalLightData[input.id].lightradius);

 //   float att = 1 / pow((dist / LocalLightData[input.id].lightradius + 1), 2);
 //   float att = 1.0f / (1 + LocalLightData[input.id].lightattenuation.y * dist + dist * dist * LocalLightData[input.id].lightattenuation.z);

    float t = pow(dist / PointLightList[input.id].lightradius, 4);
    float att = pow(saturate(1 - t), 2) / (dist * dist + 1) ; // old attenuation doesn't work... it will generate hard edge


    //return float4(att*1000, att*1000, att*1000, 1.0);
  //  att = att * att;
    //return float4(att, att, att, 1.0);

    float3 final = (diff + spec) * NL * PointLightList[input.id].lightcolor.xyz * att * PointLightList[input.id].lightintensity * test;
    return float4(final, 1.0);
////    return float4(0.03,0.03,0.03, 1.0);


//  //  return float4(NorMettexture.Sample(g_sampler, uv).rgb,1.0);
//  //  return float4(input.position.x / 1900, input.position.y / 900, 0.0, 1.0);
////	return LocalLightData[input.id].lightcolor;
}