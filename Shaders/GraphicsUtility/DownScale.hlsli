//A head file for mipmap chain code
// approach do gaussian filter first, then do subsample
#define BlockSize 16
#define gaussinwindow 5

Texture2D SrcTexture : register(t0);

#define SRGB_NO_ALPHA_MASK  // temporary for test
#ifdef GAUSSION_FILTER
static float gaussweight[5][5] =
{
    1.0,4.0,7.0,4.0,1.0,
    4.0,16.0,26.0,16.0,4.0,
    7.0,26.0,41.0,26.0,7.0,
    4.0, 16.0, 26.0, 16.0, 4.0,
    1.0, 4.0, 7.0, 4.0, 1.0,
};
#endif

#ifdef SRGB_NO_ALPHA_MASK
RWTexture2D<float4> DesTexture : register(u0);
#endif

[numthreads(BlockSize, BlockSize, 1)]
void CSMain(uint gid : SV_GroupIndex, uint3 tid : SV_DispatchThreadID)
{
#ifdef BOXFILTER
#endif
}