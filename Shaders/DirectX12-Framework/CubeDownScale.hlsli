//A head file for mipmap chain code
// approach do gaussian filter first, then do subsample

#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define PI 3.14159

#define SRGB_A
#define DISC_FILTER
/**************************

combiniation : data type + filter type + address mode

data type:

#define SRGB_A  // 4 channel
#define RGBA_LINEAR // can use in hdr mode or other....

filter type:
#define DISC_FILTER
#define GAUSSIAN_FILTER



example SRGBA+BOX_FILTER

**************************/


#define BlockSize 16
#define gaussinwindow 5
#define FOURCHANNELDATA 


static float3 faceup[6] =
{
    float3(0.0, 1.0f, 0.0f),
    float3(0.0, 1.0f, 0.0f),
    float3(0.0, 0.0f, -1.0f),
    float3(0.0, 0.0f, 1.0f),
    float3(0.0, 1.0f, 0.0f),
    float3(0.0, 1.0f, 0.0f)
};


cbuffer Camera : register(b0)
{
    //uint SrcSliceNum;
    CubeCamera cubCamera; // Texture level of source mip
}

cbuffer mipdata : register(b1)
{
    //uint SrcSliceNum;
    uint SrcMipLevel; // Texture level of source mip
}

//#define RGBA_NORMAL     // temporary for test
//#define GAUSSIAN_FILTER  // temporary for test, the very basic way to down scale


//defined(SRGB_ALPHA_MASK)||defined(SRGB_A)||defined(RGBA_LINEAR)||defined(SRGB_ALPHA_TRANSPARENCY)||defined(RGBA_NORMAL_VECTOR) // all 4 channel data




#if defined(SRGB_A)||defined(RGBA_LINEAR)
TextureCube<float4> SrcTexture : register(t0);
RWTexture2DArray<float4> DesTexture : register(u0);


#endif

SamplerState BiSampler : register(s0);




#if defined(DISC_FILTER)
void discfilterandwrite(float4 data[9], uint2 pos, uint face)
{
    float4 res = float4(0.0f, 0.0f, 0.0f, 0.0f);
    #if defined(SRGB_A)
    [unroll]
    for (uint i = 0; i < 9; ++i)
    {
        res += pow(data[i], 2.2f);
    }
    res /= 9.0f;
    DesTexture[uint3(pos, face)] = pow(res, 1.0f / 2.2f);
    # endif
}

# endif



#if defined(GAUSSIAN_FILTER)
//static float gaussweight[5][5] =
//{
//    1.0,4.0,7.0,4.0,1.0,
//    4.0,16.0,26.0,16.0,4.0,
//    7.0,26.0,41.0,26.0,7.0,
//    4.0, 16.0, 26.0, 16.0, 4.0,
//    1.0, 4.0, 7.0, 4.0, 1.0,
//};
//static float totalinv = 1.0f/273.0f;

static float gaussweight[3][3] =
{
    0.0625f,0.125f,0.0625f,
    0.125f,0.25f,0.125f,
    0.0625f, 0.125f, 0.0625f
};
static float totalinv = 1.0f;
static const int gausswidth = 3;
void gaussianfilterandwrite(float4 data[3][3], uint2 pos)
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
#if defined(SRGB_A)
   
    for (int u = 0; u < gausswidth; ++u)
    {
        for (int v = 0; v < gausswidth; ++v)
        {
            data[u][v].xyz = pow(data[u][v].xyz, 2.2f);
            result += data[u][v] * gaussweight[u][v];
            
        }
    }
    result *= totalinv;
    result.xyz = pow(result.xyz, 1.0 / 2.2f);
    DesTexture[uint3(pos, SrcSliceNum)] = result;
#endif
#if defined(SRGB_ALPHA_MASK)
  
    float totalweight = 0.0f;
    for (int u = 0; u < gausswidth; ++u)
    {
        for (int v = 0; v < gausswidth; ++v)
        {
            data[u][v].xyz = pow(data[u][v].xyz, 2.2f);
            result += data[u][v] * gaussweight[u][v]*data[u][v].w;
            totalweight+=gaussweight[u][v]*data[u][v].w;
                   
        }
    }
    result /= totalweight;
    totalweight *= totalinv;
    result.xyz = pow(result.xyz, 1.0 / 2.2f);
    result.w = step(0.5f, totalweight);
    DesTexture[uint3(pos, SrcSliceNum)] = result;
#endif
#if defined(RGBA_LINEAR)
    for (int u = 0; u < gausswidth; ++u)
    {
        for (int v = 0; v < gausswidth; ++v)
        {
            result += data[u][v] * gaussweight[u][v];
            
        }
    }
    result *= totalinv;
    DesTexture[uint3(pos, SrcSliceNum)] = result;
#endif

#if defined(RGBA_NORMAL)
    for (int u = 0; u < gausswidth; ++u)
    {
        for (int v = 0; v < gausswidth; ++v)
        {
            result += (data[u][v] * 2.0 - 1.0) * gaussweight[u][v];
            
        }
    }
    result *= totalinv;
    result.xyz = normalize(result.xyz);
    result.xyz = result.xyz * 0.5f + 0.5f;
    DesTexture[uint3(pos, SrcSliceNum)] = result;
#endif
}


#endif




float3 getViewPos(float2 uv,uint face)
{
    float4 projcoord;
    projcoord.xy = uv;
    projcoord.y = 1.0 - projcoord.y;
    projcoord.xy = projcoord.xy * 2.0f - 1.0f;
    projcoord.z = 1.0f;
    projcoord.w = 1.0f;

    float4 vpos = mul(cubCamera.projectionviewinverse[face], projcoord);
    vpos.xyzw /= vpos.w;
    return vpos.xyz;
}


[numthreads(BlockSize, BlockSize, 1)]
void CSMain(uint3 gid : SV_GroupID, uint3 tid : SV_DispatchThreadID) // tid is the position of out put texture
{
    int3 pos = tid;
    pos.z = 0;
    uint3 dim;
    uint totalmip;
    uint face = gid.z;

    SrcTexture.GetDimensions(SrcMipLevel, dim.x, dim.y, totalmip);
    float2 fdim = dim.xy;

    float2 uv = float2(tid.xy);
    uv += float2(0.5f, 0.5f);
    uv /= fdim;
    float3 vpos = getViewPos(uv, face);
    float len = length(vpos);
    float3 vdir = normalize(vpos);
    float angleoffset = (PI / 2.0f) / fdim.x; // assume the cube is square, this angle will not get nearby pix, since angles offset is actully different in different ss position
    // get radius
    float radius = tan(angleoffset) * len;

    float3 tangent = normalize(cross(faceup[face], vdir));
    float3 bitangent = normalize(cross(vdir, tangent));

#ifdef DISC_FILTER
    float discangleoffset = (2 * PI) / 8.0f;

#if defined(SRGB_A)||defined(RGBA_LINEAR)// all 4 channel data
    float4 data[9];
#endif


    // no mether what type of data in source input, still going to real them all
    float phi = 0.0f;
    [unroll]
    for (uint i = 0; i < 8; ++i)
    {

        float3 view = vpos + cos(phi) * tangent * radius + sin(phi) * bitangent * radius;
        data[i+1] = SrcTexture.SampleLevel(BiSampler, view, SrcMipLevel);
        phi += discangleoffset;
    }
    data[0] = SrcTexture.SampleLevel(BiSampler, vpos, SrcMipLevel);
    discfilterandwrite(data, pos.xy, face);
#endif


}