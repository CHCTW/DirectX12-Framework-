#define PI 3.14159
struct PointLightData
{
    float4x4 lightview[6];
    float4x4 lightproj;
    float4 lightposition;
    float4 lightcolor;
    float lightradius;
    float lightintensity;
    float2 padding;
};
#define CASCADEDCOLOR 1u
#define SMSRUSED 2u
struct DirectionLightData  // should be the same with directionlight.h
{
    float4x4 lightView;
    float4x4 lightProj;
    float4x4 lightProjtoViewProj;
    float4 boxMax;
    float4 boxMin;
    float4x4 lightSliceProj[8]; // maximum cascade shadow map num
    float4x4 lightSliceProjtoViewProj[8];
    float4 sliceBoxMaxandZ[8];
    float4 sliceBoxMinandZ[8];
    float4 lightDirection;
    float4 lightColor;
    float virtualDistance;
    float lightIntensity;
    uint sliceCount;
    uint debugmask;
};
float3 Fresnel(float3 F0, float HV)
{
    return F0 + (1.0 - F0) * pow(2, (-5.55473 * HV - 6.98316) * HV);
//	return F0 + (1.0 - F0)*pow(1 - HV, 5.0f);
}


float Distribution(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

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

