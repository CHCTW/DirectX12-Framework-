#include "../GraphicsUtility/Lightfunctions.hlsl"
#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define GROUPSIZE 32
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};



Texture2D DepthTexture : register(t0);
// setup for 5 level first
RWTexture2D<float2> HiZ0 : register(u0);
RWTexture2D<float2> HiZ1 : register(u1);
RWTexture2D<float2> HiZ2 : register(u2);
RWTexture2D<float2> HiZ3 : register(u3);
RWTexture2D<float2> HiZ4 : register(u4);
RWTexture2D<float2> HiZ5 : register(u5);
groupshared float maxZ[GROUPSIZE][GROUPSIZE];
groupshared float minZ[GROUPSIZE][GROUPSIZE];
[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, 

void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 pos = id.xy;
    uint2 dim;
    DepthTexture.GetDimensions(dim.x, dim.y);
    float ProjectionA = camera.back / (camera.back - camera.front);
    float ProjectionB = (-camera.back * camera.front) / (camera.back - camera.front);


    // load all linear z to shared memoery, due to floating point has more persiion from 0~1 normalize the depth with far distance
    maxZ[tid.x][tid.y] = ProjectionB / (DepthTexture[pos].r - ProjectionA) / camera.back;
    minZ[tid.x][tid.y] = maxZ[tid.x][tid.y];
    HiZ0[pos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]); // store value to level 0

    GroupMemoryBarrierWithGroupSync();

    // start to geneerate min,max mipmap
        
    uint2 nextpos = id.xy / 2;
    uint offset = 1;
    if(all(tid%2==0)) // only even number need to do the calculation
    {
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ1[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
    }
    GroupMemoryBarrierWithGroupSync();


    nextpos = nextpos / 2;
    offset *= 2;
    if (all(tid % 4 == 0)) // only multiples  of 4 that is &0011  need to do the calculation
    {
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ2[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
    }
    GroupMemoryBarrierWithGroupSync();

    nextpos = nextpos / 2;
    offset *= 2;
    if (all(tid % 8 == 0)) // only multiples  of 8 that is &0111  need to do the calculation
    {
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ3[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
    }
    GroupMemoryBarrierWithGroupSync();
    nextpos = nextpos / 2;
    offset *= 2;
    if (all(tid % 16 == 0)) // only multiples  of 16 that is &1111  need to do the calculation
    {
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ4[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
    }
    GroupMemoryBarrierWithGroupSync();
    nextpos = nextpos / 2;
    offset *= 2;
    if (all(tid % 32 == 0)) // only multiples  of 16 that is &1111  need to do the calculation
    {
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ5[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
    }

    
}