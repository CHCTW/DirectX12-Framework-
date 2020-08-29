#include "GraphicsUtility/GraphicsUtility.hlsl"
#include "GraphicsUtility/LightFunctions.hlsl"
#include "GraphicsUtility/Math.hlsl"
#define MaxLightCount 1024
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
StructuredBuffer<PointLightData> PointLightDataList : register(t0);
Texture2D DepthTexture : register(t1);
// the first element represtn the last filled data
RWStructuredBuffer<uint> PointLightIndexList : register(u0);
RWStructuredBuffer<uint> BlockLightCount : register(u1);
StructuredBuffer<SceneData> sceneData : register(t2);
groupshared uint minZ;
groupshared uint maxZ;
groupshared uint minPosX;
groupshared uint maxPosX;
groupshared uint minPosY;
groupshared uint maxPosY;
groupshared uint lightCount;
groupshared uint localLightIndices[MaxLightCount];
groupshared uint globalOffset;
void getViewPosition(uint2 pos, uint2 dim, out float3 vpos)
{
    float2 uv = float2(pos.x,pos.y);
    uv += float2(0.5, 0.5);
    uv = uv / (float2) dim;
    float depth = DepthTexture[pos.xy].r;
    float4 projcoord;
    projcoord.xy = uv;
    projcoord.y = 1.0 - projcoord.y;
    projcoord.xy = projcoord.xy * 2.0f - 1.0f;
    projcoord.z = depth;
    projcoord.w = 1.0f;
    float4 v = mul(camera.projinverse, projcoord);
    v.xyzw /= v.w;
    vpos = v.xyz;
}
float getSignedDistance(in float3 plane, in float3 pos)
{
    return dot(plane, pos);
}
[numthreads(BLOCKSIZE, BLOCKSIZE, 1)]
void CSMain(uint3 groupId : SV_GroupId, uint localIndex : SV_GroupIndex, uint3 threadId : SV_DispatchThreadID)
{

    uint2 dim;
    DepthTexture.GetDimensions(dim.x, dim.y);
    uint widthBlockNumber = (dim.x + BLOCKSIZE-1) / BLOCKSIZE;
    uint globalIndex = groupId.y * widthBlockNumber + groupId.x;
    float2 uv = threadId.xy;
    uv = uv / dim;
    float3 vpos;
    if (localIndex == 0)
    {
        maxZ = 0xff7fffff;
        minZ = 0;
        minPosX = 0x7f7fffff;
        maxPosX = 0;
        minPosY = 0x7f7fffff;
        maxPosY = 0;
        lightCount = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    uint2 boundPos = threadId.xy;
    boundPos.x = min(boundPos.x, dim.x-1);
    boundPos.y = min(boundPos.y, dim.y-1);
    getViewPosition(boundPos.xy, dim, vpos);
    float yOffset = camera.back * tan(camera.fov / 2);
    float xOffset = yOffset / camera.ratio;
    xOffset += 1.0f;
    xOffset += 1.0f;
    float3 corners[4];
    if (threadId.x < dim.x && threadId.y < dim.y)
    {
        // min max depth cal
        uint ori;
        if (DepthTexture[boundPos.xy].r != 1.0)
        {
            InterlockedMax(minZ, asuint(vpos.z), ori);
            InterlockedMin(maxZ, asuint(vpos.z), ori);
            // make sure it's always positive
            float2 offsetXY = float2(vpos.x + xOffset, vpos.y + yOffset);
            InterlockedMax(maxPosX, asuint(offsetXY.x));
            InterlockedMin(minPosX, asuint(offsetXY.x));
            InterlockedMax(maxPosY, asuint(offsetXY.y));
            InterlockedMin(minPosY, asuint(offsetXY.y));
        }
        
    }
    GroupMemoryBarrierWithGroupSync();
    uint2 topleft = uint2(groupId.x * BLOCKSIZE, groupId.y * BLOCKSIZE);
    uint2 topright = uint2(min(topleft.x + BLOCKSIZE - 1, dim.x - 1),topleft.y);
    uint2 botright = uint2(topright.x, min(topright.y + BLOCKSIZE - 1, dim.y - 1));
    uint2 botleft = uint2(topleft.x, min(topleft.y + BLOCKSIZE - 1, dim.y - 1));
    getViewPosition(topleft, dim, corners[0]);
    getViewPosition(topright, dim, corners[1]);
    getViewPosition(botright, dim, corners[2]);
    getViewPosition(botleft, dim, corners[3]);

    uint offset = globalIndex * sceneData[0].pointLightNumber;
    float3 planes[4];
    float3 box[2];
    box[0].x = asfloat(minPosX)-xOffset;
    box[1].x = asfloat(maxPosX) - xOffset;
    box[0].y = asfloat(minPosY) - yOffset;
    box[1].y = asfloat(maxPosY) - yOffset;
    box[0].z = asfloat(minZ);
    box[1].z = asfloat(maxZ);
    float minz = asfloat(minZ);
    float maxz = asfloat(maxZ);

    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        planes[i] = normalize(cross(corners[i], corners[(i + 1) % 4]));

    }
    for (uint lightIndex = localIndex; lightIndex < sceneData[0].pointLightNumber; lightIndex += THREADSIZE)
    {
        float4 lightpos = PointLightDataList[lightIndex].lightposition;
        float r = PointLightDataList[lightIndex].lightradius+0.1f;
        float3 lightviewpos = mul(camera.view, lightpos).xyz;
        if (AlignBoxisSphereIntsec(box[0], box[1], lightviewpos,r)/*&&
            getSignedDistance(planes[0], lightviewpos) < r &&
        getSignedDistance(planes[1], lightviewpos) <r &&
        getSignedDistance(planes[2], lightviewpos) < r &&
        getSignedDistance(planes[3], lightviewpos) < r  &&
        (lightviewpos.z + r > minz) && (lightviewpos.z - r < maxz)*/)
        {
            uint localOffset = 0;
            InterlockedAdd(lightCount, 1, localOffset);
            localLightIndices[localOffset] = lightIndex;
        }
    }

    
    GroupMemoryBarrierWithGroupSync();

    if (localIndex==0)
    {
        //uint offset = globalIndex * sceneData[0].pointLightNumber;
        uint size;
        uint endIndex;
        PointLightIndexList.GetDimensions(endIndex, size);
        endIndex -= 1;
        InterlockedAdd(PointLightIndexList[0], lightCount, globalOffset);
        BlockLightCount[globalIndex*2] = lightCount;
        BlockLightCount[globalIndex * 2 + 1] = globalOffset;
    }
    GroupMemoryBarrierWithGroupSync();
    for (uint j = localIndex; j < lightCount; j += THREADSIZE)
    {
        PointLightIndexList[globalOffset + j+1] = localLightIndices[j];
    }

}