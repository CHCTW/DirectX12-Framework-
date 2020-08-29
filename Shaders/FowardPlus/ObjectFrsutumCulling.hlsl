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

bool inSideFrustum(float4 p)
{
    float r;
    [unroll]
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
AppendStructuredBuffer<Command> VisCommands : register(u0); 
StructuredBuffer<Object> ObjectList : register(t0);
StructuredBuffer<Matrices> MatricesList : register(t1);
StructuredBuffer<Mesh> MeshList : register(t2);

[numthreads(256, 1, 1)]
void CSMain(uint globalId : SV_DispatchThreadID)
{
   
    uint id = globalId; 
    uint2 dim;
    ObjectList.GetDimensions(dim.x, dim.y);
    if (id < dim.x)  
    {
        float4 cornor[8];
        float3 min;
        float3 max;
        uint meshid;
        uint matid;
        meshid = ObjectList[id].meshid;
        matid = ObjectList[id].matricesid;
        float4 wmin = float4(MeshList[meshid].min.xyz, 1.0);
        float4 wmax = float4(MeshList[meshid].max.xyz, 1.0);
       
        generateCorner(wmin, wmax, cornor, MatricesList[matid].model);
        generateAlignBox(cornor, min, max);
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
}