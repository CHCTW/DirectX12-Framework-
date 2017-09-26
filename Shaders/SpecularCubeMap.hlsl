#define PI  3.14159265359
cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 view;
    float4x4 proj;
    float3 eye;
    float padding;
    float4x4 viewinverse;
    float4x4 projinverse;
};
cbuffer RoughnessBuffer : register(b1)
{
    float roughness;
};

TextureCube cube : register(t0);
SamplerState g_sampler : register(s0);
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}
float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}


PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL)
{
    PSInput result;
    result.normal = -position.xyz;
    result.position = mul(view, float4(position*80000 , 1.0f));
    result.position = mul(proj, result.position);
    result.position.z = result.position.w;
//	result.normal = normal;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{




    float3 specular = float3(0.0,0.0,0.10);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, input.normal));
    up = normalize(cross(input.normal, right));
    float3 N = normalize(input.normal);
    //return float4(1.0, 1.0, 0.0, 1.0);
    uint raycount = 4096;

    float weight = 0.0f;
   
    for (uint i = 0; i < raycount;i++)
    {
        float2 Xi = Hammersley(i, raycount);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0 * dot(N, H) * H - N);
        float NL = max(dot(N, L), 0.0);
        if (NL>0.0)
        {
            specular += cube.SampleLevel(g_sampler, L, 0).rgb * NL;
            weight += NL;
        }
    }


     float3 final = specular / (float) weight;
  //  final = final / (1 + final);
    return float4(final, 1.0);

 //   return float4(raycount, 1.0, 1.0, 1.0);
//     return cube.Sample(g_sampler, input.normal);


	//return float4(input.normal,1.0f);
}
