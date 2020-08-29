#include "Math.hlsl"
#define GROUPSIZE 16
cbuffer noiseconst : register(b0)
{
    float frequency;
    float valuescale;
    int repeatvalue;
    float seed; // any value > 0
    float2 offset; //0~1
};
globallycoherent RWTexture2D<float> NoiseTexture : register(u0);

[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, so the approach is invoke group with width, since width is large than height
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 dim;
    NoiseTexture.GetDimensions(dim.x, dim.y);
    if (id.x < dim.x && id.y < dim.y)  // make sure it is inside
    {
        float2 fdim = dim;
        float2 fid = id.xy;
        // 0~1
        fid /= fdim;
        // offset 
        fid += offset;
        // wrap back if large than 1
       // fid = frac(fid);

        NoiseTexture[id.xy]
         = perlinnoise2d(
        fid.x * frequency , fid.y * frequency, seed, repeatvalue) * valuescale;
       // NoiseTexture[id.xy]
    }

    
}