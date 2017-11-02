
cbuffer CameraBuffer : register(b0)
{
    float4x4 view;
    float4x4 proj;
    float4 mPosition;
    float4x4 mViewInverse;
    float4x4 mProjInverse;
    float4 mFrustumPlane[6];
}
struct Command
{
    uint2 address;
    uint2 indexarguemnt1;
    uint3 indexarguemnt2;
    uint padding;  // important!! always remember with 64 address, there could be a 4 byte end padding at the end.
};
struct InstanceData
{
    float4x4 model;
    float4x4 normal;
    float4 min;
    float4 max;
    float3 color;
    float paddingend[21]; 

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
        r = dot(p, mFrustumPlane[i]);
        if(r<0.0)
            return false;
    }
    return true;
}

AppendStructuredBuffer<Command> VisCommands : register(u0); 
StructuredBuffer<Command> TotalCommands : register(t0);
StructuredBuffer<InstanceData> InstancedBuffer : register(t1);
[numthreads(1024, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
   
    uint id = groupId.x * 1024 + groupIndex;
    uint2 dim;
    TotalCommands.GetDimensions(dim.x, dim.y);
    if (id < dim.x)
    {
        float4 wmin = mul(InstancedBuffer[id].model, float4(InstancedBuffer[id].min.xyz, 1));
        float4 wmax = mul(InstancedBuffer[id].model, float4(InstancedBuffer[id].max.xyz, 1));
        
        float4 cornor[8];
        generateCorner(wmin, wmax, cornor);

        bool result = false;

        bool draw = false;

        for (int i = 0; i < 8; ++i)
        {
            draw = draw | inSideFrustum(cornor[i]);
        }

        if (draw)
        VisCommands.Append(TotalCommands[id.x]);
    }

}