#include "Math.hlsl"
#define GROUPSIZE 16
cbuffer noiseconst : register(b0)
{
    int octaves;
    float persistance;
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
        float f = frequency;
        float v = 1.0f;
        float r = repeatvalue;
        float total = 0.0f;
        float result = 0.0f;
        for (int i = 0; i < octaves; ++i)
        {
            result += perlinnoise2d(
        fid.x * f, fid.y * f, seed, r) * v;
            total += v;
            v *= persistance;
            f *= 2;
            r *= 2;
        }
        result /= total;
        result *= valuescale;
        NoiseTexture[id.xy]
         = result;
    }

    
}