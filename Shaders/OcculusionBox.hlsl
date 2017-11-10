cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
};
cbuffer InstanceData : register(b1)
{
    float4x4 transform; // 128 byte
    float4x4 normal;
    float4 min;
    float4 max;
    float3 color;
    float paddingend[21];


};
cbuffer Cmd : register(b2)
{
    uint cmdindex;
};
struct cmd
{
    uint index;
};
RWStructuredBuffer<cmd> OcclusionRes : register(u0);

// all corner point with
static float3 cubepointax[8] = 
{
    float3(1.0f, 1.0f, 1.0f),
    float3(0.0f, 1.0f, 1.0f),
    float3(0.0f, 0.0f, 1.0f),
    float3(1.0f, 0.0f, 1.0f),

    float3(1.0f, 1.0f, 0.0f),
    float3(0.0f, 1.0f, 0.0f),
    float3(0.0f, 0.0f, 0.0f),
    float3(1.0f, 0.0f, 0.0f),


} ;
static uint indexlist[36] = 
{
    0,2,1,0,3,2,  // back
    4,5,6,4,6,7, // front
    0,4,7,0,7,3, // right
    1,6,5,1,2,6, // left
    0,1,5,0,5,4, // top
    2,3,6,3,7,6 // bot
};
float3 getCubePoint(uint index)
{
    float3 maxcom = cubepointax[indexlist[index]];
    float3 mincom = float3(1.0, 1.0, 1.0) - maxcom;
    return maxcom * max.xyz + mincom * min.xyz;

}
struct PSInput
{
	float4 position : SV_POSITION;
    float3 color : COLOR;
    nointerpolation uint index : CMDINDEX;
};
PSInput VSMain(uint vid: SV_VertexID)
{
	PSInput result;
    result.position = mul(transform, float4(getCubePoint(vid)*1.1, 1.0f)); // generatet cube point from box
    result.position = mul(view, result.position);
	result.position = mul(proj,result.position);
    result.color = cubepointax[indexlist[vid]];
    result.index = cmdindex;
	return result;
}
[earlydepthstencil]
void PSMain(PSInput input)
{
//    if (input.index==0)
 //   {
    
        OcclusionRes[input.index].index = 1;
      //  OcclusionRes[input.index].index.x = 1;
 //   }
 //   OcclusionRes[input.index].index.y = 0;
   // OcclusionRes[0].index = 1;
//    return float4(cmdindex * 0.5,1.0,1.0, 1.0f);
}
