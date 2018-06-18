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
Texture2D VIZTexture : register(t8);
RWTexture2D<float3> Reflection : register(u0);
sampler trisample : register(s0);


float WeightCaculate(float center,float2 cyrclerange,float2 hizrange,float2 pos,int mip)
{

    // x : min depth, y : max depth
    //return cyrclerange.y-hizrange.x;
  /* float midofhiz = (hizrange.x + hizrange.y) / 2.0f;
    if (cyrclerange.x > hizrange.y+2.0f || cyrclerange.y < (hizrange.x - 2.0f))
        return 0.0f;
    
    else
    {*/
        return VIZTexture.SampleLevel(trisample, pos, mip).x;
    //}

   
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
            for (int i = 0; i < 7; ++i)
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
                    float mipChannel = clamp(log2(radius * maxsize), 0.0f, 9.0f);
                float3 color = HDRTexture.SampleLevel(trisample, samplePos.xy, mipChannel).xyz;
                float2 range = HiZTexture.SampleLevel(trisample, samplePos.xy, mipChannel).yx * camera.back;
                weight = WeightCaculate(centerdepth, nearfardepth, range, samplePos, mipChannel);
                totalColor += float4(color*weight, weight);
                if (totalColor.a>=1.0f)
                    break;
                adjacentLength -= 2.0f * radius;
                if (adjacentLength <= 0.0f)
                    break;

               // totalColor.xyz += float3(weight, weight, weight);
            }
            if (totalColor.w==0.0f)
                totalColor.w = 1.0f;
            totalColor.xyz /= totalColor.w;
            theta = RoughnessToConeAngle(1.0f) * 0.5f;
            K = k1;

            float3 diffColor = HDRTexture.SampleLevel(trisample, tracepoint.xy, 9).xyz;
            //float4 diffColor = float4(0.0f, 0.0f, 0.0f, 0.0f);



            float3 F0 = float3(0.04, 0.04, 0.04);
            float metallic = GBufferTextures[2][pos].a;
            float3 albedo = pow(GBufferTextures[1][pos].rgb, 2.2);
            float3 normal = decode(GBufferTextures[0][pos].rg);
            float3 N = normal;
            float3 V = normalize(-vpos.xyz);
            float NV = max(dot(N, V), 0.0f);
            F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F
            float3 FRefect = float3(0.5, 0.5, 0.5);
            FRefect = lerp(FRefect, albedo, float3(sqrt(metallic), sqrt(metallic), sqrt(metallic)));


            float3 F = F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1 - NV, 5.0f);
            float3 Ks = F; // diffuse color 
            float3 Kd = 1 - Ks;

            Kd *= 1.0 - metallic;

            float2 BRDF = BRDFIntergrate.SampleLevel(trisample, float2(NV, roughness), 0).rg;
            diffColor.xyz = (diffColor.xyz * Kd * albedo) / PI;
            //totalColor.xyz *= Ks;
            float3 specular = FRefect * totalColor.rgb * (Ks * BRDF.x + BRDF.y);
            float2 ndc = tracepoint.xy * 2.0f - float2(1.0f, 1.0f);
            float distocenter = length(ndc);
            float fadeOnPerpendicular = saturate(lerp(0.0f, 1.0f, saturate(tracepoint.w * 4.0f)));
            float fadeOnRoughness = saturate(lerp(0.0f, 1.0f, (1.0f - roughness) *1.0));
            float fadepara = clamp(distocenter - 0.5, 0.0f, 0.5f);
            fadepara = (0.5 - fadepara)/0.5f;
            Reflection[pos] = (specular.xyz) * fadepara * fadeOnRoughness;
           // Reflection[pos] = float3(totalColor.a, totalColor.a, totalColor.a);

        }
        else
        {
            // trace faile case, should use enviroment map case...? what if we already have IBL??
            Reflection[pos] = float4(0.0f, 0.0f, 0.0f, 0.0f);

        }
            

        

    }
}