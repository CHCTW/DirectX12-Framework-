#include "Math.hlsl"
#define GROUPSIZE 256
cbuffer threashold : register(b0)
{
    float bloomthreshold;
};
Texture2D hdr : register(t0);
globallycoherent RWTexture2D<float4> bloom : register(u0);
sampler bilinear : register(s0);
static const float4 graysacelintensity = { 0.299f, 0.587f, 0.114f, 0.0f };
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
        
        float4 pixel = hdr.SampleLevel(bilinear, ((float2) id + 0.5f) / (float2) dim, 0);
        float4 orpix = pixel;
        float orinensity = dot(graysacelintensity, pixel) - bloomthreshold;
        orinensity = saturate(orinensity);
        pixel = pixel / (1 + pixel);
        float intensity = max(dot(graysacelintensity, pixel) - bloomthreshold, 0.0f);
         
        // get emmerisve datat that is over certain value, also down sample
        bloom[id.xy] = orpix * orinensity;
  //      DeviceMemoryBarrierWithGroupSync();
     //     bloom[id.yx] = float4(1.0, 0.0, 0.0, 0.0);

    //DeviceMemoryBarrierWithGroupSync();
    //AllMemoryBarrier();
     //   DeviceMemoryBarrierSybnc

      }
 //   AllMemoryBarrierWithGroupSync(); // wait for all data write in
  

 //   cache[tid.x] = bloom[id.xy - uint2(gaussion.radius, 0)]; // load previous part
 //   if (tid.x > GROUPSIZE - 1 - gaussion.radius * 2) // load right part
 //       cache[tid.x + (gaussion.radius) * 2] = bloom[id.xy + float2(gaussion.radius, 0.0)];
 //   GroupMemoryBarrierWithGroupSync(); // wait for all thread done loading to cache

 //   float4 horres = float4(0.0, 0.0, 0.0, 0.0f);
 //   for (uint i = 0; i < gaussion.radius * 2 + 1; ++i)
 //   {



 //       uint entry = i / 4;
 //       uint channel = i % 4;


 //       horres += (cache[tid.x + i] *
 //       gaussion.weight[entry][channel]);
 //   }
 ////   horres /= (float) (total);
 //   //DeviceMemoryBarrier(); // wait all res has been calculate
 //   DeviceMemoryBarrierWithGroupSync();
 //   bloom[id.xy] = horres + float4(1.0, 0.0, 0.0, 0.0);
 //   bloom[id.yx] = float4(1.0, 0.0, 0.0, 0.0);
//    AllMemoryBarrierWithGroupSync();




//    cache2[tid.x] = bloom[id.yx - uint2(0, gaussion.radius)]; // load ver blur res
//    if (tid.x > GROUPSIZE - 1 - gaussion.radius * 2) // load right part
//        cache2[tid.x + (gaussion.radius) * 2] = bloom[id.yx + float2(0.0, gaussion.radius)];
//    GroupMemoryBarrierWithGroupSync();
//    DeviceMemoryBarrierWithGroupSync();
//    float4 verres = float4(0.0, 0.0, 0.0, 0.0f);

//    uint start = id.y - gaussion.radius;
//        for (uint j = 0; j < gaussion.radius * 2 + 1; ++j)
//    {
//        float w = abs(abs((int) i - rad) - rad);
           
//        total += w;


//        uint entry = j / 4;
//        uint channel = j % 4;
//        int offset = (int) j - (int) gaussion.radius;
//        verres += (cache2[tid.x + j] * gaussion.weight[entry][channel]);

//     //   horres += (bloom[uint2(id.x, start+ j)] * gaussion.weight[entry][channel]);
//    }

//  DeviceMemoryBarrierWithGroupSync();
   

    
// //   AllMemoryBarrierWithGroupSync();

////    bloom[id.xy] = verres;
//    bloom[id.yx] = verres;



//    bloom[id.yx] = horres;

    
}