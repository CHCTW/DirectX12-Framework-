#include "../GraphicsUtility/GraphicsUtility.hlsl"
#include "../GraphicsUtility/LightFunctions.hlsl"
cbuffer DirectoinLightBuffer : register(b0)
{
    DirectionLightData directionlight;
};
cbuffer Objecetid : register(b1)
{
    uint ojectindex;
};

StructuredBuffer<Object> objectList : register(t0);
StructuredBuffer<Matrices> matrixList : register(t1);

struct GSInput
{
	float4 position : SV_POSITION;
};
struct GSOutput
{
    float4 position : SV_POSITION;
    uint targetindex : SV_RenderTargetArrayIndex;
    
};
GSInput VSMain(float3 position : POSITION)
{
    GSInput result;

    result.position = mul(matrixList[objectList[ojectindex].matricesid].model, float4(position, 1.0f));
    result.position = mul(directionlight.lightView, result.position);
  //  result.position = mul(directionlight.lightProj, result.position);
 //   result.position.xy *= 5;
 //   result.position.z *= -1;
 //   result.lgihtindex = ShadowLightIndices[shadowlightindexoffset + instanceid];
    return result;
}

[maxvertexcount(24)]
void GSMain(triangle GSInput input[3], inout TriangleStream<GSOutput> stream)
{
    GSOutput outres;
    [unroll]
    for (uint slice = 0; slice < directionlight.sliceCount; ++slice) //choose casacade shadow map here
    {
    
        outres.targetindex = slice;
        [unroll]
        for (int i = 0; i < 3; ++i) 
        {
            outres.position = mul(directionlight.lightSliceProj[slice], input[i].position);
            stream.Append(outres);
        }
        stream.RestartStrip();

    }
}