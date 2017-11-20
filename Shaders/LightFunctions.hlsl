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
static float3 cubepoints[8] =
{
    float3(1.0f, 1.0f, 1.0f),
    float3(-1.0f, 1.0f, 1.0f),
    float3(-1.0f, -1.0f, 1.0f),
    float3(1.0f, -1.0f, 1.0f),

    float3(1.0f, 1.0f, -1.0f),
    float3(-1.0f, 1.0f, -1.0f),
    float3(-1.0f, -1.0f, -1.0f),
    float3(1.0f, -1.0f, -1.0f),


};
static uint indexlist[36] =
{
    0, 2, 1, 0, 3, 2, // back
    4, 5, 6, 4, 6, 7, // front
    0, 4, 7, 0, 7, 3, // right
    1, 6, 5, 1, 2, 6, // left
    0, 1, 5, 0, 5, 4, // top
    2, 3, 6, 3, 7, 6 // bot
};
static float2 quadpoints[4] =  // 
{
    float2(-1.0,1.0),
    float2(-1.0, -1.0),
    float2(1.0, 1.0),
    float2(1.0, -1.0),
};
static uint quadindexlist[6] =
{
    0,1,2,
    2,1,3
};

float3 getUnitCubePoint(in uint index)
{
    return cubepoints[indexlist[index]];
}
float2 getUnitQuadPoint(in uint index)
{
    return quadpoints[quadindexlist[index]];
}
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

