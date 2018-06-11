#define PI  3.14159265359
struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
PSInput VSMain(uint id : SV_VertexID)
{
	PSInput result;
	result.uv = float2((id << 1) & 2, id & 2);
	result.position = float4(result.uv  * float2(2, -2) + float2(-1, 1), 0, 1);


	return result;
}
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
float GeometrySchlickGGX(float NV, float roughness)
{
    float r = roughness;
    float k = (r * r) / 2.0;

    float nom = NV;
    float denom = NV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(float NV, float NL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NV, roughness);
    float ggx1 = GeometrySchlickGGX(NL, roughness);
    return ggx1 * ggx2;
}
float2 PSMain(PSInput input) : SV_TARGET
{

    float roughness = saturate(input.uv.y);
    float NV = saturate(input.uv.x);
    float3 V = float3(sqrt(1.0f - (NV * NV)), 0.0f, NV);
    float3 N = float3(0.0f, 0.0f, 1.0f);
    float A = 0; 
    float B = 0;
    uint raycount = 1024;
    for (uint i = 0u; i < raycount; i++)
    {
        float2 Xi = Hammersley(i, raycount);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        

        float3 L = 2.0 * dot(V, H) * H - V;
        float NL = saturate(L.z);
        float NH = saturate(H.z);
        float VH = saturate(dot(V, H));
        if(NL>0)
        {
            float G = GeometrySmith(NV, NL, roughness);
            float G_vis = (G * VH) / (NH * NV);
            float Fc = pow(1 - VH, 5);
            A += (1 - Fc) * G_vis;
            B += Fc * G_vis;
        }
    }
    A = A / raycount;
    B = B / raycount;
 //   if (N = 0.01f)
    // return float2(1.0, 0.0);
   return float2(A, B);
//	return g_texture.Sample(g_sampler, input.uv);
}
