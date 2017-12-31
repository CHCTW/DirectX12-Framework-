#include "GraphicsUtility/GraphicsUtility.hlsl"
#include "GraphicsUtility/LightFunctions.hlsl"
#include "GraphicsUtility/Math.hlsl"
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
struct Command
{
    VertexBufferView position;
    VertexBufferView normal;
    VertexBufferView uv;
    VertexBufferView tangent;
    VertexBufferView bitangent;
    IndexBufferView index;
    uint objindex;
    uint2 indexarguemnt1;
    uint3 indexarguemnt2;
};

struct ShadowDrawCmd  // only need position
{
    VertexBufferView position;
    IndexBufferView index;
    uint objindex;
    uint lightindexoffset;
    uint2 indexarguemnt1;
    uint3 indexarguemnt2;
    uint padding;
};


bool inSideFrustum(float4 p)
{
    float r;
//    [unroll]
    for (int i = 0; i < 6; ++i)
    {
        r = dot(p, camera.mFrustumPlane[i]);
        if(r<0.0)
            return false;
    }
    return true;
}
bool InSidePlane(in float4 cornor[8], uint index)
{
    float r = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        r = dot(cornor[i], camera.mFrustumPlane[index]);
        if (r >= 0.0)
            return true;
    }
    return false;
}
//bool CircleInSideFrustum(in float3 position, in float radius)
//{

//}
AppendStructuredBuffer<Command> VisCommands : register(u0); 
RWStructuredBuffer<uint> ShadowLightIndices : register(u1);  // idnex 0 is work as the counter of the total lights in this list, remember to offset 1 when right light index
AppendStructuredBuffer<ShadowDrawCmd> ShadowCommands : register(u2);
StructuredBuffer<Object> ObjectList : register(t0);
StructuredBuffer<Matrices> MatricesList : register(t1);
StructuredBuffer<Mesh> MeshList : register(t2);
StructuredBuffer<PointLightData> PointLightList : register(t3);
groupshared float4 cornor[8];
groupshared float3 min;
groupshared float3 max;
groupshared uint validlightnum;
groupshared uint validlightlist[256];
groupshared uint shadowlightindexoffset;
groupshared uint meshid;
groupshared uint matid;
[numthreads(256, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex, uint threadID : SV_GroupThreadID)
{
   
    uint id = groupId.x; // use group number to point the out the object 
    uint2 dim;
    uint2 lightdim;
    ObjectList.GetDimensions(dim.x, dim.y);
    PointLightList.GetDimensions(lightdim.x, lightdim.y);
    if (threadID.x==0)  // for the first thread, need to caculate the cube points for whole group
    {
        meshid = ObjectList[id].meshid;
        matid = ObjectList[id].matricesid;
        float4 wmin = float4(MeshList[meshid].min.xyz, 1.0);
        float4 wmax = float4(MeshList[meshid].max.xyz, 1.0);
        validlightnum = 0;
       
        generateCorner(wmin, wmax, cornor, MatricesList[matid].model);
        generateAlignBox(cornor, min, max);
        if (id==0)
        {
            InterlockedMax(ShadowLightIndices[0], 1u);
            InterlockedMin(ShadowLightIndices[0], 1u);

         //   ShadowLightIndices[0] = 1; // reset valid light list
        }
    }
    GroupMemoryBarrierWithGroupSync(); // wait for the cube point data
   // DeviceMemoryBarrierWithGroupSync();

    if (threadID.x < lightdim.x) // total light count
    {
        // check the light will show on the screen
        bool valid;



        valid = AlignBoxisSphereIntsec(min, max, PointLightList[threadID.x].lightposition.xyz, PointLightList[threadID.x].lightradius);
     //   valid = true;
        if(valid)
        {
            uint index;
            InterlockedAdd(validlightnum, 1,index); // add to vliad light num;
            validlightlist[index] = threadID.x;  // use index to recored the light index in shared memory
        }

    }
    GroupMemoryBarrierWithGroupSync(); // wait for all lights caculate their result, that is we get all lights that may lit this object

    if(threadID.x==1) // 0,1 doesn't metter just need only one thread to get light index offset and atomiclly add
    {
        if(validlightnum>0) // there are lights contant with this object
        {
            InterlockedAdd(ShadowLightIndices[0], validlightnum, shadowlightindexoffset); // add lights that contat with shadow list
            // also need to add indirect shadow draw command here, instance num  = validlightnum
            // need to send shadowlightindexoffset as  root constant to pick the light inofmration

            ShadowDrawCmd shadow;
            shadow.position = MeshList[meshid].position;
            shadow.index = MeshList[meshid].index;
            shadow.objindex = id;
            shadow.lightindexoffset = shadowlightindexoffset;
            shadow.indexarguemnt1.x = MeshList[meshid].indexCount;
            shadow.indexarguemnt1.y = validlightnum;
            shadow.indexarguemnt2.x = MeshList[meshid].startIndex;
            shadow.indexarguemnt2.y = 0;
            shadow.indexarguemnt2.z = 0;
            shadow.padding = 0;
            ShadowCommands.Append(shadow);
        }
    }

    if (threadID.x == 0)  //  frustum culling to decied whetehr should darw on screen
    {

        bool result = false;
        bool draw = true;

        for (int i = 0; i < 6; ++i)
        {
            draw = draw & InSidePlane(cornor, i);
        }

        if (draw)
        {
            Command cmd;
            cmd.position = MeshList[meshid].position;
            cmd.normal = MeshList[meshid].normal;
            cmd.uv = MeshList[meshid].uv;
            cmd.tangent = MeshList[meshid].tangent;
            cmd.bitangent = MeshList[meshid].bitangent;
            cmd.index = MeshList[meshid].index;
            cmd.objindex = id;
            cmd.indexarguemnt1.x = MeshList[meshid].indexCount;
            cmd.indexarguemnt1.y = 1;
            cmd.indexarguemnt2.x = MeshList[meshid].startIndex;
            cmd.indexarguemnt2.y = 0;
            cmd.indexarguemnt2.z = 0;
            VisCommands.Append(cmd);
        }
       
    }


    GroupMemoryBarrierWithGroupSync();  // wait for thread 1 to get the offset
    if(threadID.x < validlightnum)
    {
        ShadowLightIndices[shadowlightindexoffset + threadID.x] = validlightlist[threadID.x];
    }

}