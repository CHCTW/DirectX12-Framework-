//A head file for mipmap chain code
// approach do gaussian filter first, then do subsample

/**************************

combiniation : data type + filter type + address mode

data type:

#define SRGB_A  // 4 channel
#define SRGB_ALPHA_MASK // 4 channel
#define SRGB_ALPHA_TRANSPARENCY // 4 channel
#define R_LINEAR // 1 channel not rgba data
#define RGBA_NORMAL // 4 channel
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
#define FOURCHANNELDATA 

cbuffer mipdata : register(b0)
{
    uint SrcSliceNum;
    uint SrcMipLevel; // Texture level of source mip
}

//#define RGBA_NORMAL     // temporary for test
//#define GAUSSIAN_FILTER  // temporary for test, the very basic way to down scale
//#define CLAMP

//defined(SRGB_ALPHA_MASK)||defined(SRGB_A)||defined(RGBA_LINEAR)||defined(SRGB_ALPHA_TRANSPARENCY)||defined(RGBA_NORMAL_VECTOR) // all 4 channel data




#if defined(SRGB_ALPHA_MASK)||defined(SRGB_A)||defined(RGBA_LINEAR)||defined(SRGB_ALPHA_TRANSPARENCY)||defined(RGBA_NORMAL) 
RWTexture2DArray<float4> SrcTexture : register(u0);
RWTexture2DArray<float4> DesTexture : register(u1);

#endif





#ifdef BOX_FILTER
static int2 offset[4] =
{
    int2(0, 0),
    int2(0, 1),
    int2(1, 0),
    int2(1, 1),
};
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
#if defined(SRGB_ALPHA_MASK)
    float total = 0.0f;
    float alphatotal = 0.0f; 
    [unroll]
    for (uint i = 0; i < 4; ++i)  // move rgb sapce to linear space
    {
        data[i].xyz = pow(data[i].xyz, 2.2f);
        result += data[i];
        total += data[i].w;

    }

    
    result *=0.25;
    result.xyz = pow(result.xyz, 1.0 / 2.2f);
    result.w = step(2.0+0.001,total);
    DesTexture[uint3(pos, SrcSliceNum)] = result;
    
#endif


#if defined(RGBA_LINEAR)
    [unroll]
    for (uint i = 0; i < 4; ++i)  // move rgb sapce to linear space
    {
        result += data[i];
    }
    result *= 0.25;
    DesTexture[uint3(pos,SrcSliceNum)] = result;
#endif


#if defined(RGBA_NORMAL)
    [unroll]
    for (uint i = 0; i < 4; ++i)  // move rgb sapce to linear space
    {
        result += (data[i] * 2.0 - 1.0);
    }
    result *= 0.25;
    result.xyz = normalize(result.xyz);
    result.xyz = result.xyz * 0.5f + 0.5f;
    DesTexture[uint3(pos,SrcSliceNum)] = result;
#endif

}

#endif



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




uint2 addressPos(int2 pos, int2 dim)
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





[numthreads(BlockSize, BlockSize, 1)]
void CSMain(uint gid : SV_GroupIndex, uint3 tid : SV_DispatchThreadID) // tid is the position of out put texture
{
      int3 pos = tid;
      pos.z = 0;
    uint3 dim;
   
   

#ifdef BOX_FILTER


#if defined(SRGB_ALPHA_MASK)||defined(SRGB_A)||defined(RGBA_LINEAR)||defined(SRGB_ALPHA_TRANSPARENCY)||defined(RGBA_NORMAL)// all 4 channel data
    float4 data[4];
    SrcTexture.GetDimensions(dim.x, dim.y, dim.z);
#endif


    // no mether what type of data in source input, still going to real them all
    [unroll]
    for (uint i = 0; i < 4; ++i)
        data[i] = SrcTexture[uint3(addressPos(pos.xy * 2 + offset[i], dim.xy), SrcSliceNum)]; // read all four nearby data, .mips[][]not support
    boxfilterandwrite(data, tid.xy);
#endif


#ifdef GAUSSIAN_FILTER
#if defined(SRGB_ALPHA_MASK)||defined(SRGB_A)||defined(RGBA_LINEAR)||defined(SRGB_ALPHA_TRANSPARENCY)||defined(RGBA_NORMAL)// all 4 channel data
    float4 data[gausswidth][gausswidth];
    SrcTexture.GetDimensions(dim.x, dim.y, dim.z);
#endif
    for (int u = -gausswidth / 2; u <= gausswidth/2; ++u)
    {
        for (int v = -gausswidth / 2; v <= gausswidth/2; ++v)
        {
            data[u + gausswidth / 2][v + gausswidth/2] = SrcTexture[uint3(addressPos(pos.xy * 2 + int2(u, v), dim.xy), SrcSliceNum)];
        }

    }
    gaussianfilterandwrite(data, tid.xy);

#endif
}