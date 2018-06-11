#include "../GraphicsUtility/Lightfunctions.hlsl"
#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define GROUPSIZE 16
#define MAXITERATION 40.0f
#define MAXHDRLEVEL 4
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
// decode normal
// albedo + rougbess
//  unused + metlaic
//  depth
Texture2D GBufferTextures[4] : register(t0);
Texture2D<float2> HiZTexture : register(t4);
Texture2D HDRTexture : register(t5);
Texture2D TraceTexture : register(t6);
Texture2D BRDFIntergrate : register(t7);
RWTexture2D<float3> Reflection : register(u0);
sampler trisample : register(s0);


float WeightCaculate(float center,float2 cyrclerange,float2 hizrange)
{

    // x : min depth, y : max depth
    //return cyrclerange.y-hizrange.x;
    float midofhiz = (hizrange.x + hizrange.y) / 2.0f;
    if (cyrclerange.x > hizrange.y || cyrclerange.y < (hizrange.x - 2.0f))
        return 0.0f;
    if (center <= hizrange.y && center >= (hizrange.x - 2.0f))
        return 0.6f;
    else
        return 0.15f;

   
    //return 1.0f;

}

[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, 
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 pos = id.xy;
    uint2 dim;

    GBufferTextures[0].GetDimensions(dim.x, dim.y);
    float2 fdim = dim;
    //float depth = DepthTexture[pos].r;
    [branch]
    if(all(pos<dim)) 
    {
        float4 tracepoint = TraceTexture[pos];
        [branch]
        if (tracepoint.z!=0)
        {

            float2 uv = (float2(pos) + float2(0.5f, 0.5f)) / fdim;
            float maxsize = max(fdim.x, fdim.y);
            float depth = GBufferTextures[3][pos].r;
            float4 projcoord;
            projcoord.xy = uv;
            projcoord.y = 1.0 - projcoord.y;
            projcoord.xy = projcoord.xy * 2.0f - 1.0f;
            projcoord.z = depth;
            projcoord.w = 1.0f;
            
            float4 vpos = mul(camera.projinverse, projcoord);
            vpos.xyzw /= vpos.w;
            float roughness = GBufferTextures[1][pos].a;
            float theta = RoughnessToConeAngle(roughness) * 0.5f;
            


            float2 deltaP = tracepoint.xy - uv.xy;
            float adjacentLength = length(deltaP);
            float2 adjacentUnit = normalize(deltaP);
            float k0 = -1.0f / (vpos.z);
            float k1 = -1.0f / (tracepoint.z);
            float deltaK = k1 - k0;
            float dK = deltaK / adjacentLength;
            float K = k1;
            float4 totalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
            float remainingAlpha = 1.0f;
            float oppositeleng = 0.0f;
            float radius = 0.0f;
            float weight = 0.0f;
            // only trace one point now
            for (int i = 0; i < 1; ++i)
            {
                oppositeleng = 2.0f * tan(theta) * adjacentLength;
                float a2 = oppositeleng * oppositeleng;
                float fh2 = 4.0f * adjacentLength * adjacentLength;
                radius = (oppositeleng * (sqrt(a2 + fh2) - oppositeleng)) / (4.0f * adjacentLength);

                //radius = oppositeleng/2.0f;

                float2 samplePos = uv.xy + adjacentUnit * (adjacentLength);
                float2 nearfardepth = float2(k0 + dK * (adjacentLength - 2*radius), k0 + dK * (adjacentLength));
                float centerdepth = k0 + dK * (adjacentLength - radius);
                nearfardepth = 1.0f / nearfardepth;
                centerdepth = 1.0f / centerdepth;
                if (nearfardepth.x > nearfardepth.y)
                    nearfardepth.xy = nearfardepth.yx;
                //lindeardepth = 1.0 / lindeardepth;
                //float range = 1.0f/K - lindeardepth;
                    float mipChannel = clamp(log2(radius * maxsize)-1, 0.0f, 9);
                float3 color = HDRTexture.SampleLevel(trisample, samplePos.xy, mipChannel).xyz;
                float2 range = HiZTexture.SampleLevel(trisample, samplePos.xy, mipChannel).yx * camera.back;
                weight = WeightCaculate(centerdepth,nearfardepth, range);
                totalColor += float4(color, weight);
                if (totalColor.a>=1.0f)
                    break;
                adjacentLength -= 2.0f * radius;
                if (adjacentLength <= 0.0f)
                    break;

               // totalColor.xyz += float3(weight, weight, weight);
            }

            theta = RoughnessToConeAngle(1.0f) * 0.5f;
            K = k1;

            float3 diffColor = HDRTexture.SampleLevel(trisample, tracepoint.xy, 8).xyz;
            //float4 diffColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
            remainingAlpha = 1.0f;
            oppositeleng = 0.0f;
            radius = 0.0f;
            weight = 0.0f;



            float3 F0 = float3(0.04, 0.04, 0.04);
            float metallic = GBufferTextures[2][pos].a;
            float3 albedo = pow(GBufferTextures[1][pos].rgb, 2.2);
            float3 normal = decode(GBufferTextures[0][pos].rg);
            float3 N = normal;
            float3 V = normalize(-vpos.xyz);
            float NV = max(dot(N, V), 0.0f);
            float3 Ks = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F
            float2 BRDF = BRDFIntergrate.SampleLevel(trisample, float2(NV, roughness), 0).rg;
            diffColor.xyz = (diffColor.xyz * (1.0f - Ks) * albedo) / PI;
            //totalColor.xyz *= Ks;
            float3 specular = totalColor.xyz * (Ks * BRDF.x + BRDF.y);
            float2 ndc = tracepoint.xy * 2.0f - float2(1.0f, 1.0f);
            float distocenter = length(ndc);
            float fadeOnPerpendicular = saturate(lerp(0.0f, 1.0f, saturate(tracepoint.w * 4.0f)));
            float fadeOnRoughness = saturate(lerp(0.0f, 1.0f, (1.0f - roughness) *1.0));
            float fadepara = clamp(distocenter - 0.5, 0.0f, 0.5f);
            fadepara = (0.5 - fadepara)/0.5f;
            Reflection[pos] = (specular.xyz + diffColor.xyz) * fadepara * fadeOnRoughness * fadeOnPerpendicular;
            //Reflection[pos] = float3(weight, weight, weight);

        }
        else
        {
            // trace faile case, should use enviroment map case...? what if we already have IBL??
            Reflection[pos] = float4(0.0f, 0.0f, 0.0f, 0.0f);

        }
            

        

    }
}