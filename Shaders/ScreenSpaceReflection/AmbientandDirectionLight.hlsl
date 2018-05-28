#include "../GraphicsUtility/Lightfunctions.hlsl"
#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define GROUPSIZE 16
#define MAXSEARCH 8
static float4 debugcolor[8] =
{
    float4(1.0, 0.0, 0.0, 0.0),
    float4(0.0, 1.0, 0.0, 0.0),
    float4(0.0, 0.0, 1.0, 0.0),
    float4(1.0, 1.0, 0.0, 0.0),
    float4(0.0, 1.0, 1.0, 0.0),
    float4(0.0, 1.0, 1.0, 0.0),
    float4(1.0, 1.0, 1.0, 0.0),
    float4(0.0, 0.0, 0.0, 0.0)
};


cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
cbuffer DirectoinLightBuffer : register(b1)
{
    DirectionLightData directionlight;
};
Texture2D GBufferTextures[4] : register(t2);
// decode normal
// albedo + rougbess
//  unused + metlaic
//  depth
Texture2DArray ShadowMap : register(t0);
TextureCube skytext : register(t1);
RWTexture2D<float4> HDR : register(u0);
SamplerState pointsampler : register(s0);
[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, 
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 pos = id.xy;
   


    float3 albedo = pow(GBufferTextures[1][pos].rgb, 2.2);
    float roughness = GBufferTextures[1][pos].a;
    float3 normal = decode(GBufferTextures[0][pos].rg);
    float metallic = GBufferTextures[2][pos].a;
    float depth = GBufferTextures[3][pos].r;
    float2 uv = pos;
    float2 dim;
    HDR.GetDimensions(dim.x, dim.y);
    uv = uv / dim;
    float4 projcoord;
    projcoord.xy = uv;
    projcoord.y = 1.0 - projcoord.y;
    projcoord.xy = projcoord.xy * 2.0f - 1.0f;
    projcoord.z = depth;
    projcoord.w = 1.0f;
    float4 vpos = mul(camera.projinverse, projcoord);
    vpos.xyzw /= vpos.w;
    float4 wpos = mul(camera.projviewinverse, projcoord);
    wpos /=  wpos.w;



    float4 light = directionlight.lightDirection;
    light.w = 0.0f;
    light = mul(camera.viewinverseyranspose, light); // move light direction to view space

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F

    float3 N = normal;
    float3 V = normalize(-vpos.xyz);
    float3 L = normalize(-light.xyz);
    float3 H = normalize(L + V);
    float LH = max(dot(L, H), 0.0f);
    float NL = max(dot(L, N), 0.0f);
    float NV = max(dot(N, V), 0.0f);
    float HV = max(dot(H, V), 0.0f);



    float3 F = Fresnel(F0, HV);
    float D = Distribution(N, H, roughness);
    float3 G = GeometrySmith(NV, NL, roughness);

    float3 spec = D * F * G / (4 * NV * NL + 0.001);


    float3 Kd = 1 - F; // diffuse color 
    Kd = Kd * (1.0 - metallic);
    float3 diff = Kd * albedo / PI;

    uint slidenum = 0;

    for (uint s = 0; s < directionlight.sliceCount; ++s)
    {
        if (depth > directionlight.sliceBoxMaxandZ[s].w)
            ++slidenum;
    }

    float4x4 lightproj = mul(directionlight.lightSliceProj[slidenum], directionlight.lightView);
    float4 shadowcoord = mul(lightproj, wpos);


    float3 viewdir = wpos.xyz - camera.eye;
    float3 skycolor = skytext.SampleLevel(pointsampler, viewdir, 0).xyz * step(1.0, depth);


    shadowcoord /= shadowcoord.w;
    shadowcoord.xy = shadowcoord.xy * 0.5 + 0.5;
    shadowcoord.y = 1 - shadowcoord.y;
    float pixdepth = shadowcoord.z;
    shadowcoord.z = slidenum;
    float3 vShadowMapDims; // need to keep in sync with .cpp file
    uint ele, level;
    ShadowMap.GetDimensions(0, vShadowMapDims.x, vShadowMapDims.y, ele, level);
    
    float2 texsize = 1.0f / float2(vShadowMapDims.x, vShadowMapDims.y);


    float test = 0.0f;
    int looksize = 2;

    int2 shadowpos = int2(vShadowMapDims.x * shadowcoord.x, vShadowMapDims.y * shadowcoord.y);

    float total = 0.0f;
//    [unroll]
    for (int x = -looksize; x <= looksize; ++x)
    {
  //      [unroll]
        for (int y = -looksize; y <= looksize; ++y)
        {
            float lightdepth = ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz + float3(texsize * float2(x, y), 0.0f), 0);
            if ((pixdepth) > lightdepth + 0.005f)
                test += 1.0f;
        }

    }
    test /= (looksize * 2 + 1) * (looksize * 2 + 1);
   // test /= total;
    test = clamp(1.0f - test, 0.0f, 1.0f);


        float2 vTexelUnits = 1.0f / vShadowMapDims;





   

    if ((pixdepth) > ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz, 0).r + 0.003f)
        test = 0.0f;


////    test = exp(-120.0f * pixdepth) * exp(120.0f * ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz, 0).r);

//    test = saturate(test);
////    test = exp(-80 * pixdepth) * ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz, 0).r;



    
    float3 final = (diff + spec) * NL * directionlight.lightColor.xyz * directionlight.lightIntensity * test + albedo*0.04+skycolor;
  
  //  final = skycolor;

    if (directionlight.debugmask & CASCADEDCOLOR)
    {
        final += (debugcolor[slidenum] * 0.5f).rgb;
    }
    HDR[pos] = float4(final, 1.0);


}