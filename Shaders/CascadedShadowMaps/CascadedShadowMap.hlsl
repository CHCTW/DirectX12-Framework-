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
    float4 outpoints[3];

    [unroll]
    for (uint slice = 0; slice < directionlight.sliceCount; ++slice) //choose casacade shadow map here
    {
    
        outres.targetindex = slice;
        float3 maxpoint = directionlight.sliceBoxMinandZ[slice].xyz;
        float3 minpoint = directionlight.sliceBoxMaxandZ[slice].xyz;
        [unroll]
        for (int i = 0; i < 3; ++i)
        {
            maxpoint = max(input[i].position.xyz, maxpoint);
            minpoint = min(input[i].position.xyz, minpoint);
        }
        bool draw = all(maxpoint.xy > directionlight.sliceBoxMinandZ[slice].xy) && all(minpoint.xyz < float3(directionlight.sliceBoxMaxandZ[slice].xy, -directionlight.sliceBoxMinandZ[slice].z));

        if (draw)
        {
        
            for (int i = 0; i < 3; ++i)
            {
                outres.position = mul(directionlight.lightSliceProj[slice], input[i].position);
                outres.position = outres.position / outres.position.w;
                stream.Append(outres);
            }
            stream.RestartStrip();
        }
    
    }
}