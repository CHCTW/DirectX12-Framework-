#include "../GraphicsUtility/Math.hlsl"
#define GROUPSIZE 128
cbuffer miplevel : register(b0)
{
    uint miplevel;
};
Texture2D<float4> input : register(t0);
RWTexture2D<float4> ouput : register(u0);
SamplerState linearsampler : register(s0);
// 9-tap weigh and offset, use blinear sameling can reduce almost half of the access
static float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
static float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

float3 ToneMapACES(float3 hdr)
{
    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    return saturate((hdr * (A * hdr + B)) / (hdr * (C * hdr + D) + E));
}
[numthreads(GROUPSIZE, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    // since it's already lower mip level, we need to multiply by 2 to access higher level
    uint2 pos = id.xy*2;
    uint2 dim;
    uint levelnum;
    input.GetDimensions(miplevel, dim.x, dim.y, levelnum);
    float2 diminv = dim;
    diminv = 1.0f / diminv;
    float2 uv = (pos + float2(0.5f, 0.5f)) * diminv; // move to the center of the pixel
    if (all(uv >= 0.0f) && all(uv <= 1.0f))
    {
        
        float4 res = input.SampleLevel(linearsampler, uv, miplevel) * weight[0];
        [unroll]
        for (int i = 1; i < 3;++i)
        {
            res += input.SampleLevel(linearsampler, uv + float2(0.0f,offset[i]) * diminv, miplevel) * weight[i];
            res += input.SampleLevel(linearsampler, uv - float2(0.0f,offset[i]) * diminv, miplevel) * weight[i];

        }
        if (miplevel==0)
        {
            res.xyz = ToneMapACES(res.xyz);

        }
        ouput[id.xy] = res;
    }
    

}