#include "Math.hlsl"
#define GROUPSIZE 16
cbuffer noiseconsts : register(b0)
{
    float scale;
    float time;
    float2 offset;
    uint reverse;
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
        st *= scale;
        rst *= scale;
        //st += float2(scale / 2.0f, scale / 2.0f);
       // st = fmod(st, float(scale));

        float mindist = 1.0f;

        // i_st from 0 ~ scale should be int
        
        float2 i_st = floor(st);

        float2 ri_st = floor(rst);

   //     i_st = abs(i_st - float2(scale / 2.0f, scale / 2.0f));
    
        float2 f_st = frac(st);
        float2 rf_st = frac(rst);
       // f_st = abs(f_st - float2(0.5f, 0.5f))*2.0f;


        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                float2 neighbor = float2(float(x), float(y));
                float2 p = random2(i_st + neighbor);
                p = 0.5 + 0.5 * sin(time + 6.2831 * p);
                float2 diff = neighbor + p - (f_st);
                
                p = random2(ri_st + neighbor);
                p = 0.5 + 0.5 * sin(time + 6.2831 * p);


                float2 diff2 = neighbor + p - (rf_st);

                mindist = min(mindist, length(diff));
          //      mindist = min(mindist, length(diff2));
            }
        }

        if (reverse == 0)
            NoiseTexture[id.xy] = mindist;
        else
            NoiseTexture[id.xy] = 1.0f - pow(mindist, 2.0f);
    }

    
}
