#include "GraphicsUtility.hlsl"
#include "Lightfunctions.hlsl"
#include "Math.hlsl"


StructuredBuffer<Object> objectList: register(t0);
StructuredBuffer<Matrices> matrixList : register(t1);
StructuredBuffer<uint> ShadowLightIndices : register(t2);
StructuredBuffer<PointLightData> PointLightList : register(t3);
cbuffer RootConstant : register(b0)
{
	uint objectid;
    uint shadowlightindexoffset;
};
struct PSInput
{
    float4 position : POSITION;
    nointerpolation uint lgihtindex : LID;
};
struct GSOutput
{
    float4 position : SV_POSITION;
    float normlineardepth : L_DEPTH;
    uint targetindex : SV_RenderTargetArrayIndex;
};
PSInput VSMain(float3 position : POSITION, uint instanceid : SV_InstanceID)
{
    PSInput result;

    result.position = mul(matrixList[objectList[objectid].matricesid].model, float4(position, 1.0f));
    result.lgihtindex = ShadowLightIndices[shadowlightindexoffset + instanceid].x;
    return result;
}
[maxvertexcount(18)]
void GSMain(triangle PSInput input[3], inout TriangleStream<GSOutput> stream)
{
    GSOutput output;
    output.position = input[0].position;
    output.targetindex = 0;
    output.normlineardepth = 0;
    uint lightindex = input[0].lgihtindex;
    float4 srpoints[3];
    float3 lightpos = PointLightList[lightindex].lightposition.xyz;
    float lightrad = PointLightList[lightindex].lightradius;
    float4 maxpoint;
    float4 minpoint;
    bool draw = IsSphereTriangleIntersectBackCull(input[0].position.xyz, input[1].position.xyz, input[2].position.xyz, lightpos, lightrad);
  //  bool draw = true;
    [branch]
    if (draw)  // have traingle culling here? maybe do as a prototype
    {
        [unroll]
        for (int face = 0; face < 6; ++face)
        {
            output.targetindex = face + lightindex * 6;


          //  [unroll]
          //  for (int i = 0; i < 3; ++i)
          //  {
          //      output.position = float4(input[i].position.xyz, 1.0);
          ////          output.wposition = input[i].position.xyz;
          //      output.position = mul(PointLightList[lightindex].lightview[face], output.position);
          //      output.position = mul(PointLightList[lightindex].lightproj, output.position);
          //      srpoints[i] = output.position;
          //      output.normlineardepth = length(input[i].position.xyz - lightpos.xyz) / lightrad;
          //    //  stream.Append(output);
          //  }

            if(draw)
            {
            
             [unroll]
                for (int i = 0; i < 3; ++i)
                {
                    output.position = float4(input[i].position.xyz, 1.0);
          //          output.wposition = input[i].position.xyz;
                    output.position = mul(PointLightList[lightindex].lightview[face], output.position);
                    
                    output.position = mul(PointLightList[lightindex].lightproj, output.position);
                    float t = output.position.w;
       //             output.position /= t;
            //        output.position.w = 1.0f;

                    output.normlineardepth = length(input[i].position.xyz - lightpos.xyz) / lightrad;
                    stream.Append(output);
                }
                stream.RestartStrip();
            }
        }

    }
        
       

}

float PSMain(GSOutput input) : SV_Depth
{
    return input.normlineardepth;
}