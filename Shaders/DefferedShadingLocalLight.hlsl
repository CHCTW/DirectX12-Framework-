#define PI 3.14159f
cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 view;
    float4x4 proj;
    float3 eye;
    float padding;
    float4x4 viewinverse;
    float4x4 projinverse;
};


struct PointLightData
{
	float4x4 lightview[6];
	float4x4 lightproj;
	float4 lightposition;
	float4 lightcolor;
	float4 lightattenuation;
	float lightradius;
	float back;
	float2 padding;
};

Texture2D NorMettexture : register(t0);
Texture2D AlbRoutexture : register(t1);
Texture2D Depthtexture : register(t2);
StructuredBuffer<PointLightData> LocalLightData : register(t3);
SamplerState g_sampler : register(s0);
struct PSInput
{
	float4 position : SV_POSITION;
	uint id : ID;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, uint instanceid : SV_InstanceID)
{
	PSInput result;
//	result.position = float4(1.0, 1.0, 1.0, 1.0);
    result.position = float4(position * LocalLightData[instanceid].lightradius + LocalLightData[instanceid].lightposition.xyz, 1.0); // scale and transition the light shape
	result.position = mul(view, result.position);
	result.position = mul(proj, result.position);
	result.id = instanceid;
	return result;
}
float3 Fresnel(float3 F0, float costheta)
{
    return F0 + (1.0 - F0) * pow(1 - costheta, 5.0f);
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
float4 PSMain(PSInput input) : SV_TARGET
{
	//input.position.x/1920
    float2 uv = float2(input.position.x / 1600, input.position.y / 900);


    float3 normal = NorMettexture.Sample(g_sampler, uv).rgb;
    float metallic = NorMettexture.Sample(g_sampler, uv).a;
    float3 albedo = AlbRoutexture.Sample(g_sampler, uv).rgb;
    float roughness = AlbRoutexture.Sample(g_sampler, uv).a;
    float depth = Depthtexture.Sample(g_sampler, uv).r;
    float4 projcoord;
    projcoord.xy = uv;
    projcoord.y = 1.0 - projcoord.y;
    projcoord.xy = projcoord.xy * 2.0f - 1.0f;
    projcoord.z = depth;
    projcoord.w = 1.0f;
    float4 pos = mul(projinverse, projcoord);
//	pos.xyz = pos.xyz / pos.w;
    pos = mul(viewinverse, pos);
    pos.xyz = pos.xyz / pos.w;

//	return pos;
    float4 lightpostion = LocalLightData[input.id].lightposition;


    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F

    float3 N = normalize(normal);
    float3 V = normalize(eye - pos.xyz);
    float3 L = normalize(lightpostion.xyz - pos.xyz);
    float3 H = normalize(L + V);
    float LH = max(dot(L, H), 0.0f);
    float NL = max(dot(L, N), 0.0f);
    float NV = max(dot(N, V), 0.0f);
    float HV = max(dot(H, V), 0.0f);

//	return float4(pos.xyz, 1.0);

    float3 F = Fresnel(F0, HV);
    float D = Distribution(N, H, roughness);
    float3 G = GeometrySmith(NV, NL, roughness);

    float3 spec = D * F * G / (4 * NV * NL + 0.001);


    float3 Kd = 1 - F; // diffuse color 
    Kd = Kd * (1.0 - metallic);
    float3 diff = Kd * albedo / PI;


    float dist = length(lightpostion.xyz - pos.xyz);
 //   float att = 1.0f / (1 + LocalLightData[input.id].lightattenuation.y * dist + dist * dist * LocalLightData[input.id].lightattenuation.z);

    float att = 1.0 - (dist / LocalLightData[input.id].lightradius) * (dist / LocalLightData[input.id].lightradius); // old attenuation doesn't work... it will generate hard edge
    if (dist > LocalLightData[input.id].lightradius)
        discard;
  //  att = att * att;
    //return float4(att, att, att, 1.0);

    float3 final = (diff + spec) * NL * LocalLightData[input.id].lightcolor.xyz*att ;
    //final = final / (1 + final); // tone mapping
   // final = pow(final, 1.0f / 2.2f) * att;
//	pos.xyz = pos.xyz / 100;
//	pos.xyz = normalize(pos.xyz);
    return float4(final, 1.0);


  //  return float4(NorMettexture.Sample(g_sampler, uv).rgb,1.0);
  //  return float4(input.position.x / 1900, input.position.y / 900, 0.0, 1.0);
//	return LocalLightData[input.id].lightcolor;
}