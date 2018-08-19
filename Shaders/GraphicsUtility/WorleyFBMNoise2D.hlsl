#include "Math.hlsl"
#define GROUPSIZE 16
cbuffer noiseconsts : register(b0)
{
    float scale;
    float time;
    float2 offset;
    uint reverse;
    int octave;
};
globallycoherent RWTexture2D<float> NoiseTexture : register(u0);

[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, so the approach is invoke group with width, since width is large than height
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 dim;
    NoiseTexture.GetDimensions(dim.x, dim.y);
    if (id.x < dim.x && id.y < dim.y)  // make sure it is inside
    {
        float2 fdim = dim;
       
        float2 fid = id.xy;
        //fid += float2(0.5f, 0.5f);
        fid += offset;

        
         
        float2 st = fid / fdim;
        float2 rst = float2(0.5f, 0.5f) - st;

        st.x *= fdim.x / fdim.y;

        float total = 0.0f;
        uint r = reverse;
        float totalweight = 1.0f;
        float currentweight = 1.0f;
        float currentscale = scale;
        for (int i = 0; i < octave; ++i)
        {
        
            float2 cst = st * currentscale;
            total += worleynoise2d(cst, time, r) * currentweight;
            totalweight += currentweight;
            currentweight *= 0.5f;
            currentscale *= 2.0f;

        }
        total /= totalweight;
        NoiseTexture[id.xy] = total;
        
    }
    
}
