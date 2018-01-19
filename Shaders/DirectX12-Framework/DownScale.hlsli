//A head file for mipmap chain code
// approach do gaussian filter first, then do subsample

/**************************

combiniation : data type + filter type + address mode

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


address mode:
#define CLAMP
#define WRAP
#define MIRROR

example SRGBA+BOX_FILTER

**************************/


#define BlockSize 16
#define gaussinwindow 5

Texture2DArray SrcTexture : register(t0);
cbuffer mipdata : register(b0)
{
    uint SrcSliceNum;
    uint SrcMipLevel; // Texture level of source mip
}

//#define CLAMP
//#define SRGB_A  // temporary for test
//#define BOX_FILTER  // temporary for test, the very basic way to down scale


#ifdef BOX_FILTER
static int2 offset[4] =
{
    int2(0, 0),
    int2(0, 1),
    int2(1, 0),
    int2(1, 1),
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
RWTexture2DArray<float4> DesTexture : register(u0);
#endif
uint2 addressPos(int2 pos,int2 dim)
{
    int2 spos = pos;
 


#if defined(CLAMP)
    spos = clamp(spos, int2(0, 0), dim-int2(1,1));
#elif defined(WRAP)
     while (any(spos < int2(0,0)))  // make sure spos is > 0
        spos = spos + dim;
    spos = spos % dim;
#elif defined(MIRROR)
    while (any(spos < int2(0, 0)))  // make sure spos is > 0
        spos = spos + 2*dim; // mirror cycle is 2* size
    spos = spos % (2 * dim);
    if(spos.x>=dim.x) // mirror 
        spos.x = dim.x - (spos.x - dim.x + 1);
    if (spos.y >= dim.y) // mirror 
        spos.y = dim.y - (spos.y - dim.y + 1);
#endif
    return spos;
}

void boxfilterandwrite(float4 data[4],uint2 pos) // we decide how to dealing with differnt usage of data
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
    DesTexture[uint3(pos,SrcSliceNum)] = result;
#endif
}



[numthreads(BlockSize, BlockSize, 1)]
void CSMain(uint gid : SV_GroupIndex, uint3 tid : SV_DispatchThreadID) // tid is the position of out put texture
{
      int3 pos = tid;
      pos.z = 0;
    uint3 dim;
    SrcTexture.GetDimensions(dim.x, dim.y, dim.z);
#ifdef BOX_FILTER
    float4 data[4];

    
    // no mether what type of data in source input, still going to real them all
    [unroll]
    for (uint i = 0; i < 4; ++i)
        data[i] = SrcTexture.Load(uint4(addressPos(pos.xy * 2 + offset[i], dim.xy), SrcSliceNum, SrcMipLevel)); // read all four nearby data, .mips[][]not support
    boxfilterandwrite(data, tid.xy);
#endif
}