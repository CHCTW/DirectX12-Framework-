#include "../GraphicsUtility/Math.hlsl"
#define GROUPSIZE 256
cbuffer threashold : register(b0)
{
    float bloomthreshold;
};
Texture2D hdr : register(t0);
globallycoherent RWTexture2D<float4> bloom : register(u0);
sampler bilinear : register(s0);
static const float4 graysacelintensity = { 0.2126, 0.7152f, 0.0722f, 0.0f };
[numthreads(GROUPSIZE, 1, 1)] // block size 256 pixel, so the approach is invoke group with width, since width is large than height
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 dim;
    bloom.GetDimensions(dim.x,dim.y);
    if (id.x < dim.x  && id.y < dim.y)  // make sure it is inside
    {
        // donw sample first
        // use bilinear to get four most recent data and interpoloate

        float4 down = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float2 diminv = 1.0f / (float2) dim;
        float2 offset = 0.25f * diminv;
        float2 uv = ((float2) id + 0.5f) * diminv;
        float4 pixel = hdr.SampleLevel(bilinear, uv, 0);


        // from dx mini engine very nicely remove lone pixel with very high energy

        float4 pixel1 = hdr.SampleLevel(bilinear, uv + float2(-offset.x, -offset.y), 0);
        float4 pixel2 = hdr.SampleLevel(bilinear, uv + float2(offset.x, -offset.y), 0);
        float4 pixel3 = hdr.SampleLevel(bilinear, uv + float2(-offset.x, offset.y), 0);
        float4 pixel4 = hdr.SampleLevel(bilinear, uv + float2(offset.x, offset.y), 0);



        float luma1 = dot(graysacelintensity, pixel1); // use gray scale as intensity
        float luma2 = dot(graysacelintensity, pixel2);
        float luma3 = dot(graysacelintensity, pixel3);
        float luma4 = dot(graysacelintensity, pixel4);

        const float kSmallEpsilon = 0.0001;


        pixel1 *= max(kSmallEpsilon, luma1 - bloomthreshold) / (luma1 + kSmallEpsilon); 
        pixel2 *= max(kSmallEpsilon, luma2 - bloomthreshold) / (luma2 + kSmallEpsilon);
        pixel3 *= max(kSmallEpsilon, luma3 - bloomthreshold) / (luma3 + kSmallEpsilon);
        pixel4 *= max(kSmallEpsilon, luma4 - bloomthreshold) / (luma4 + kSmallEpsilon);

        const float kShimmerFilterInverseStrength = 1.0f;
        float weight1 = 1.0f / (luma1 + kShimmerFilterInverseStrength);
        float weight2 = 1.0f / (luma2 + kShimmerFilterInverseStrength);
        float weight3 = 1.0f / (luma3 + kShimmerFilterInverseStrength);
        float weight4 = 1.0f / (luma4 + kShimmerFilterInverseStrength);
        float weightSum = weight1 + weight2 + weight3 + weight4;  // 

         
        // get emmerisve datat that is over certain value, also down sample
       // bloom[id.xy] = orpix * orinensity;
  //      DeviceMemoryBarrierWithGroupSync();
        bloom[id.xy] = (pixel1 * weight1 + pixel2 * weight2 + pixel3 * weight3 + pixel4 * weight4) / weightSum;;

    //DeviceMemoryBarrierWithGroupSync();
    //AllMemoryBarrier();
     //   DeviceMemoryBarrierSybnc

      }

    
}