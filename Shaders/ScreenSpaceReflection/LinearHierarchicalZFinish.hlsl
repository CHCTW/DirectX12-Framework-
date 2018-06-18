#include "../GraphicsUtility/Lightfunctions.hlsl"
#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define GROUPSIZE 32

RWTexture2D<float2> HiZ5 : register(u0);
RWTexture2D<float2> HiZ6 : register(u1);
RWTexture2D<float2> HiZ7 : register(u2);
RWTexture2D<float2> HiZ8 : register(u3);
RWTexture2D<float2> HiZ9 : register(u4);

RWTexture2D<float> VIZ5 : register(u5);
RWTexture2D<float> VIZ6 : register(u6);
RWTexture2D<float> VIZ7 : register(u7);
RWTexture2D<float> VIZ8 : register(u8);
RWTexture2D<float> VIZ9 : register(u9);

groupshared float maxZ[GROUPSIZE][GROUPSIZE];
groupshared float minZ[GROUPSIZE][GROUPSIZE];
groupshared float Viz[GROUPSIZE][GROUPSIZE];
[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, 

void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 pos = id.xy;
    uint2 dim;
    HiZ5.GetDimensions(dim.x, dim.y);

    
    
// load all linear z to shared memoery, due to floating point has more persiion from 0~1 normalize the depth with far distance
    maxZ[tid.x][tid.y] = HiZ5[pos].r;
    minZ[tid.x][tid.y] = HiZ5[pos].g;
    Viz[tid.x][tid.y] = VIZ5[pos];
    GroupMemoryBarrierWithGroupSync();

// start to geneerate min,max mipmap
        
    uint2 nextpos = id.xy / 2;
    uint offset = 1;
    if (all(tid % 2 == 0)) // only even number need to do the calculation
    {
        float4 viz = float4(Viz[tid.x][tid.y], Viz[tid.x + offset][tid.y], Viz[tid.x][tid.y + offset], Viz[tid.x + offset][tid.y + offset]);
        float4 z = float4(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y], maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]) + float4(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y], minZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]);
        z /= 2.0f;
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ6[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
        z /= maxZ[tid.x][tid.y];
        viz = viz * z;
        Viz[tid.x][tid.y] = dot(float4(0.25f, 0.25f, 0.25f, 0.25f), viz);
        VIZ6[nextpos] = Viz[tid.x][tid.y];
    }
    GroupMemoryBarrierWithGroupSync();


    nextpos = nextpos / 2;
    offset *= 2;
    if (all(tid % 4 == 0)) // only multiples  of 4 that is &0011  need to do the calculation
    {
        float4 viz = float4(Viz[tid.x][tid.y], Viz[tid.x + offset][tid.y], Viz[tid.x][tid.y + offset], Viz[tid.x + offset][tid.y + offset]);
        float4 z = float4(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y], maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]) + float4(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y], minZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]);
        z /= 2.0f;
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ7[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
        z /= maxZ[tid.x][tid.y];
        viz = viz * z;
        Viz[tid.x][tid.y] = dot(float4(0.25f, 0.25f, 0.25f, 0.25f), viz);
        VIZ7[nextpos] = Viz[tid.x][tid.y];
    }
    GroupMemoryBarrierWithGroupSync();

    nextpos = nextpos / 2;
    offset *= 2;
    if (all(tid % 8 == 0)) // only multiples  of 8 that is &0111  need to do the calculation
    {
        float4 viz = float4(Viz[tid.x][tid.y], Viz[tid.x + offset][tid.y], Viz[tid.x][tid.y + offset], Viz[tid.x + offset][tid.y + offset]);
        float4 z = float4(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y], maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]) + float4(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y], minZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]);
        z /= 2.0f;
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ8[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
        z /= maxZ[tid.x][tid.y];
        viz = viz * z;
        Viz[tid.x][tid.y] = dot(float4(0.25f, 0.25f, 0.25f, 0.25f), viz);
        VIZ8[nextpos] = Viz[tid.x][tid.y];
    }
    GroupMemoryBarrierWithGroupSync();

    nextpos = nextpos / 2;
    offset *= 2;
    if (all(tid % 16 == 0)) // only multiples  of 8 that is &0111  need to do the calculation
    {
        float4 viz = float4(Viz[tid.x][tid.y], Viz[tid.x + offset][tid.y], Viz[tid.x][tid.y + offset], Viz[tid.x + offset][tid.y + offset]);
        float4 z = float4(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y], maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]) + float4(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y], minZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]);
        z /= 2.0f;
        maxZ[tid.x][tid.y] = max(max(maxZ[tid.x][tid.y], maxZ[tid.x + offset][tid.y]), max(maxZ[tid.x][tid.y + offset], maxZ[tid.x + offset][tid.y + offset]));
        minZ[tid.x][tid.y] = min(min(minZ[tid.x][tid.y], minZ[tid.x + offset][tid.y]), min(maxZ[tid.x][tid.y + offset], minZ[tid.x + offset][tid.y + offset]));
        HiZ9[nextpos] = float2(maxZ[tid.x][tid.y], minZ[tid.x][tid.y]);
        z /= maxZ[tid.x][tid.y];
        viz = viz * z;
        Viz[tid.x][tid.y] = dot(float4(0.25f, 0.25f, 0.25f, 0.25f), viz);
        VIZ9[nextpos] = Viz[tid.x][tid.y];
    }
    

    
}