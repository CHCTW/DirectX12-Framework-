#include "GraphicsUtility.hlsl"
#include "LightFunctions.hlsl"
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
    IndexBufferView index;
    uint objindex;
    uint2 indexarguemnt1;
    uint3 indexarguemnt2;
};

void generateCorner(float4 min,float4 max,out float4 cornor[8])
{
  //  float4 c[8];
    cornor[0] = float4(max.x, max.y, max.z, 1);
    cornor[1] = float4(max.x, max.y, min.z, 1);
    cornor[2] = float4(max.x, min.y, max.z, 1);
    cornor[3] = float4(min.x, max.y, max.z, 1);
    cornor[4] = float4(max.x, min.y, min.z, 1);
    cornor[5] = float4(min.x, max.y, min.z, 1);
    cornor[6] = float4(min.x, min.y, max.z, 1);
    cornor[7] = float4(min.x, min.y, min.z, 1);
    //corner = c;
}
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
AppendStructuredBuffer<Command> VisCommands : register(u0); 
RWStructuredBuffer<uint> ShadowLightIndices : register(u1);
StructuredBuffer<Object> ObjectList : register(t0);
StructuredBuffer<Matrices> MatricesList : register(t1);
StructuredBuffer<Mesh> MeshList : register(t2);
StructuredBuffer<PointLightData> PointLightList : register(t3);
[numthreads(256, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
   
    uint id = groupId.x * 256 + groupIndex;
    uint2 dim;
    ObjectList.GetDimensions(dim.x, dim.y);
    if (id < dim.x)
    {
        uint meshid = ObjectList[id].meshid;
        uint matid = ObjectList[id].matricesid;
        float4 wmin = mul(MatricesList[matid].model, float4(MeshList[meshid].min.xyz, 1));
        float4 wmax = mul(MatricesList[matid].model, float4(MeshList[meshid].max.xyz, 1));
        
        float4 cornor[8];
        generateCorner(wmin, wmax, cornor);

        bool result = false;
        bool draw = true;

        //for (int i = 0; i < 8; ++i)
        //{
        //    draw = draw & inSideFrustum(cornor[i]);
        //}

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