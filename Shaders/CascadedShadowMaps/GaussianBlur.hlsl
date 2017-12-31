#include "../GraphicsUtility/Math.hlsl"
#define GROUPSIZE 256
cbuffer direction : register(b1)
{
    uint direction;
};
cbuffer threashold : register(b0)
{
    GaussionData gaussiandata;
};
Texture2D input : register(t0);
RWTexture2D<float4> ouput : register(u0);
static int2 directions[2] =
{
    int2(1, 0),
    int2(0, 1)
};
groupshared float4 cache[MAXRADIUS * 2 + GROUPSIZE]; // data store area 
[numthreads(GROUPSIZE, 1, 1)] // block size 256 pixel, 
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 block = gid.x * directions[direction] + gid.y * (int2(1, 1) - directions[direction]);
    uint2 loc = (block.xy * directions[direction] * GROUPSIZE) + block.xy * (int2(1, 1) - directions[direction]) + tid.x * directions[direction];
    //uint2 dim;
    //input.GetDimensions(dim.x, dim.y);
    //if (id.xy - gaussiandata.radius * directions[direction] < dim.xy )
    //{
    
    cache[tid.x] = input[loc.xy - gaussiandata.radius * directions[direction]]; // load previous part
        if (tid.x > GROUPSIZE - 1 - gaussiandata.radius * 2) // load right part
        cache[tid.x + (gaussiandata.radius) * 2] = input[loc.xy + gaussiandata.radius * directions[direction]];
        GroupMemoryBarrierWithGroupSync(); // wait for all thread done loading to cache

        float4 res = float4(0.0, 0.0, 0.0, 0.0f);
      //  [unroll]
        for (uint i = 0; i < gaussiandata.radius * 2 + 1; ++i)
        {
            uint entry = i / 4;
            uint channel = i % 4;
            res += (cache[tid.x + i] *gaussiandata.weight[entry][channel]);
        }
    ouput[loc.xy] = res;
   // ouput[id.xy] = float4(1.0, 0.0, 0.0, 0.0);
//    }
}