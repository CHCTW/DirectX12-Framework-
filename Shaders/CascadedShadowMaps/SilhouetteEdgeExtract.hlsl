#include "../GraphicsUtility/Lightfunctions.hlsl"
#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define GROUPSIZE 16
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
Texture2D GBufferTextures[4] : register(t1);
// decode normal
// albedo + rougbess
//  unused + metlaic
//  depth
Texture2DArray ShadowMap : register(t0);
RWTexture2D<float4> Silhouette : register(u0);
SamplerState g_sampler : register(s0);
[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, 
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 pos = id.xy;
    //HDR[pos] = GBufferTextures[1][pos];
    float depth = GBufferTextures[3][pos].r;
    float2 uv = pos;
    float2 dim;
    Silhouette.GetDimensions(dim.x, dim.y);
    uv = uv / dim;
    float4 projcoord;
    projcoord.xy = uv;
    projcoord.y = 1.0 - projcoord.y;
    projcoord.xy = projcoord.xy * 2.0f - 1.0f;
    projcoord.z = depth;
    projcoord.w = 1.0f;
    float4 vpos = mul(camera.projinverse, projcoord);
    vpos.xyzw /= vpos.w;
    float4 wpos = mul(camera.viewinverse, vpos);
    wpos /=  wpos.w;








    uint slidenum = 0;

    for (uint s = 0; s < directionlight.sliceCount; ++s)
    {
        if(depth>directionlight.sliceBoxMaxandZ[s].w)
            ++slidenum;
    }

    
    float4 lightwcoord = mul(directionlight.lightView, wpos);
    float4 shadowcoord = mul(directionlight.lightSliceProj[slidenum], lightwcoord);
    shadowcoord /= shadowcoord.w;
    shadowcoord.xy = shadowcoord.xy * 0.5 + 0.5;
    shadowcoord.y = 1 - shadowcoord.y;
    float pixdepth = shadowcoord.z;
    shadowcoord.z = slidenum;
    float3 vShadowMapDims; // need to keep in sync with .cpp file
    uint ele, level;
    ShadowMap.GetDimensions(0, vShadowMapDims.x, vShadowMapDims.y, ele, level);
    float2 texsize = 1.0f / float2(vShadowMapDims.x, vShadowMapDims.y);


    float test = 1.0f;

    float center = pixdepth>ShadowMap.SampleLevel(g_sampler, shadowcoord.xyz, 0).r + 0.008f;
    float left = pixdepth>ShadowMap.SampleLevel(g_sampler, shadowcoord.xyz + float3(texsize * float2(-1.0, 0.0), 0.0f), 0).r + 0.008f;
    float right = pixdepth>ShadowMap.SampleLevel(g_sampler, shadowcoord.xyz + float3(texsize * float2(1.0, 0.0), 0.0f), 0).r + 0.008f;
    float top = pixdepth>ShadowMap.SampleLevel(g_sampler, shadowcoord.xyz + float3(texsize * float2(0.0, -1.0), 0.0f), 0).r + 0.008f;
    float bot = pixdepth>ShadowMap.SampleLevel(g_sampler, shadowcoord.xyz + float3(texsize * float2(0.0, 1.0), 0.0f), 0).r + 0.008f;


    float4 disc = abs(float4(left, right, bot, top) - center) ;
	
    float2 dxdy = 0.75f + (-disc.xz + disc.yw) * 0.25f;
    dxdy = (1.0 -center) * dxdy * step(1.0f, float2(dot(disc.xy, 1.0f), dot(disc.zw, 1.0f)));

    Silhouette[pos] = float4(dxdy, center, 0);

}