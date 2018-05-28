#include "../GraphicsUtility/GraphicsUtility.hlsl"
#include "../GraphicsUtility/LightFunctions.hlsl"
#include "../GraphicsUtility/Math.hlsl"
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
cbuffer DirectoinLightBuffer : register(b1)
{
    DirectionLightData directionlight;
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
    uint2 indexarguemnt1;
    uint3 indexarguemnt2;
    //uint padding;
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
AppendStructuredBuffer<ShadowDrawCmd> ShadowCommands : register(u1);
StructuredBuffer<Object> ObjectList : register(t0);
StructuredBuffer<Matrices> MatricesList : register(t1);
StructuredBuffer<Mesh> MeshList : register(t2);
groupshared float4 cornor[8];
groupshared float3 min;
groupshared float3 max;
groupshared uint meshid;
groupshared uint matid;
[numthreads(1, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex, uint threadID : SV_GroupThreadID)
{
   
    uint id = groupId.x; // use group number to point the out the object 
    uint2 dim;
    ObjectList.GetDimensions(dim.x, dim.y);

    float4 lightviewcornor[8];
    float3 lightviewmin;
    float3 lightviewmax;

    if (threadID.x==0)  // for the first thread, need to caculate the cube points for whole group
    {
        meshid = ObjectList[id].meshid;
        matid = ObjectList[id].matricesid;
        float4 wmin = float4(MeshList[meshid].min.xyz, 1.0);
        float4 wmax = float4(MeshList[meshid].max.xyz, 1.0);
       
        generateCorner(wmin, wmax, cornor, MatricesList[matid].model);
        generateAlignBox(cornor, min, max);

        //generateCorner(wmin, wmax, lightviewcornor, mul(directionlight.lightView, MatricesList[matid].model));
        //generateAlignBox(lightviewcornor, lightviewmin, lightviewmax);

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

        // generate shdowcommand
        ShadowDrawCmd shadowcmd;
        shadowcmd.position = MeshList[meshid].position;
        shadowcmd.index = MeshList[meshid].index;
        shadowcmd.objindex = id;
        shadowcmd.indexarguemnt1.x = MeshList[meshid].indexCount;
        shadowcmd.indexarguemnt1.y = 1;
        shadowcmd.indexarguemnt2.x = MeshList[meshid].startIndex;
        shadowcmd.indexarguemnt2.y = 0;
        shadowcmd.indexarguemnt2.z = 0;
        ShadowCommands.Append(shadowcmd);
    }


}