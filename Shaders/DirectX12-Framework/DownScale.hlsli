//A head file for mipmap chain code
// approach do gaussian filter first, then do subsample

/**************************

combiniation : data type + filter type

data type:

#define SRGB_A  // 4 channel
#define SRGB_ALPHA_MASK // 4 channel
#define SRGB_ALPHA_TRANSPARENCY // 4 channel
#define R_LINEAR // 1 channel not rgba data
#define RGBA_NORMAL_VECTOR // 4 channel
#define RGBA_LINEAR // can use in hdr mode or other....

filter type:
#define BOX_FITER
#define GAUSSIAN_FILTER

example SRGBA+BOX_FILTER

**************************/


#define BlockSize 16
#define gaussinwindow 5

Texture2D<float4> SrcTexture : register(t0);
cbuffer mipdata : register(b0)
{
    uint SrcMipLevel; // Texture level of source mip
}


//#define SRGB_A  // temporary for test
//#define BOX_FILTER  // temporary for test, the very basic way to down scale


#ifdef BOX_FILTER
static float2 offset[4] =
{
    float2(0.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f),
};

#elif GAUSSION_FILTER
static float gaussweight[5][5] =
{
    1.0,4.0,7.0,4.0,1.0,
    4.0,16.0,26.0,16.0,4.0,
    7.0,26.0,41.0,26.0,7.0,
    4.0, 16.0, 26.0, 16.0, 4.0,
    1.0, 4.0, 7.0, 4.0, 1.0,
};
#endif

#if defined(SRGB_ALPHA_MASK)||defined(SRGB_A) // all 4 channel data
RWTexture2D<float4> DesTexture : register(u0);
#endif

void boxfilterandwrite(float4 data[4],uint2 pos)
{
    float4 result = float4(0.0f,0.0f,0.0f,0.0f);
#if defined(SRGB_A)
    [unroll]
    for (uint i = 0; i < 4; ++i)  // move rgb sapce to linear space
    {
        data[i].xyz = pow(data[i].xyz, 2.2f);
        result += data[i];
    }
    result *= 0.25;
    result.xyz = pow(result.xyz, 1.0/2.2f);
    DesTexture[pos] = result;
#endif
}



[numthreads(BlockSize, BlockSize, 1)]
void CSMain(uint gid : SV_GroupIndex, uint3 tid : SV_DispatchThreadID) // tid is the position of out put texture
{
#ifdef BOX_FILTER
    float4 data[4];
// no mether what type of data in source input, still going to real them all
    [unroll]
    for (uint i = 0; i < 4; ++i)
        data[i] = SrcTexture.Load(uint3(tid.xy * 2 + offset[i], SrcMipLevel)); // read all four nearby data, .mips[][]not support
    
    boxfilterandwrite(data, tid.xy);
    //SrcTexture.[]
#endif
}