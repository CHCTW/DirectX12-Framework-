#include "../GraphicsUtility/GraphicsUtility.hlsl"
struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
Texture2D littexture : register(t0);
Texture2D bloomtexure : register(t1);
Texture2D reflectuv: register(t2);
Texture2D reflection : register(t3);
SamplerState g_sampler : register(s0);

static float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
static float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

PSInput VSMain(uint id : SV_VertexID)
{
	PSInput result;
	result.uv = float2((id << 1) & 2, id & 2);
	result.position = float4(result.uv  * float2(2, -2) + float2(-1, 1), 0, 1);


	return result;
}
float3 ToneMapACES(float3 hdr)
{
    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    return saturate((hdr * (A * hdr + B)) / (hdr * (C * hdr + D) + E));
}
float4 PSMain(PSInput input) : SV_TARGET
{

    float4 farpos = mul(camera.projviewinverse, float4((input.uv - 0.5f) * 2.0f, 0.5f, 1.0f));
    farpos /= farpos.w;

    float3 dir = farpos.xyz - camera.eye;


	//return float4(input.uv,0.0f,0.0f);
    float3 refuvvalid = reflection.Sample(g_sampler, input.uv).xyz;
    
    float3 final = littexture.Sample(g_sampler, input.uv).xyz + bloomtexure.Sample(g_sampler, input.uv).xyz + reflection.Sample(g_sampler, input.uv).xyz;
    //float3 final = littexture.Sample(g_sampler, refuvvalid.xy).xyz * refuvvalid.z;
    //final = reflection.Sample(g_sampler, input.uv).xyz;

    float3 reflect = float3(0.0, 0.0, 0.0);
    

   // final = final / (1.0f + final);
    final = ToneMapACES(final);
    final = pow(final, 1.0f / 2.2f);

   // final = reflection.Sample(g_sampler, input.uv).xyz;
  //  final = pow(final, 2.2f);

   // if (refuvvalid.x >= 1.0f || refuvvalid.x <= 0.0f)
   //     refuvvalid = float3(0.0f, 0.0f, 0.0f);
    //final = refuvvalid;
    //final.z = 0;

    return float4(final, 1.0);
}
