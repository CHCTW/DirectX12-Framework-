#include "GraphicsUtility/GraphicsUtility.hlsl"
#define PI 3.14159
struct InstancedInformation
{
    Matrices matrices;
    Material material;
};


cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float3 eye;
};


cbuffer SpotLightData : register(b1)
{
    float4x4 lightview;
    float4x4 lightproj;
    float4 lightpostion;
    float4 lightcolor;
    float lightradius;
    float lightintensity;
    float lightconeangle;
    float padding;
};
cbuffer HeightScale : register(b2)
{
    float height;
};

StructuredBuffer<InstancedInformation> instances: register(t0);
Texture2D blockbase[4] : register(t1);
SamplerState loopsample : register(s0);
struct VSOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 wposition : WPOSITION;
    float2 uv : TEXTCOORD;
    float3 tangent : TANGET;
	uint id : ID;
};





VSOutput VSMain(float3 position : POSITION, float3 normal : NORMAL, float2 uv:TEXTCOORD, float3 tangent : TANGENT,uint instanceid : SV_InstanceID)
{
	VSOutput result;

	result.position = mul(instances[instanceid].matrices.model, float4(position, 1.0f));
	result.wposition = result.position.xyz;

 //   result.wposition = result.wposition + blockbase[2].SampleLevel(loopsample, uv,0).xyz;

	result.position = mul(view, result.position);
	result.position = mul(proj,result.position);
	
    result.uv = uv;
    result.tangent = mul(instances[instanceid].matrices.normal, float4(tangent, 0.0f)).xyz;
	result.normal = mul(instances[instanceid].matrices.normal, float4(normal, 0.0f)).xyz;
	result.id = instanceid;
	return result;
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

float4 PSMain(VSOutput input) : SV_TARGET
{


    float3 normaltext = normalize(blockbase[1].Sample(loopsample, input.uv) * 2.0 - 1.0);
    float3 normalobject = normalize(input.normal);
    float3 tan = normalize(input.tangent);
    float3 bitan = cross(tan,normalobject);



        float3 normal = normaltext.x * tan + normaltext.y * bitan + normaltext.z * normalobject;

        float roughness = blockbase[3].Sample(loopsample, input.uv).r;
    float metallic = blockbase[1].Sample(loopsample, input.uv).r;


        float3 albedo = pow(blockbase[0].Sample(loopsample, input.uv), 2.2);
    //if (blockbase[0].Sample(loopsample, input.uv).w==0.0f)
    //    discard;
        float ao = 1.0f;
        
   // return float4(normal, 0.0f);
        
    


        float3 F0 = float3(0.04, 0.04, 0.04);
        F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F



        float3 N = normalize(normal);
        float3 V = normalize(eye - input.wposition);
        float3 L = normalize(lightpostion.xyz - input.wposition);
        float3 H = normalize(L + V);
        float LH = max(dot(L, H), 0.0f);
        float NL = max(dot(L, N), 0.0f);
        float NV = max(dot(N, V), 0.0f);
        float HV = max(dot(H, V), 0.0f);

        float3 F = Fresnel(F0, HV);
        float D = Distribution(N, H, roughness);
        float3 G = GeometrySmith(NV, NL, roughness);
   // return float4(F.y, F.y, F.y,1.0);


        float3 spec = D * F * G / (4 * NV * NL + 0.001);


        float3 Kd = 1 - F; // diffuse color 
        Kd = Kd * (1.0 - metallic);
        float3 diff = Kd * albedo / PI;

        float dist = length(lightpostion.xyz - input.wposition);
	    //float att = 1.0f / (1 + lightattenuation.y*dist + dist*dist*lightattenuation.z);
        float3 plightspace = normalize(mul(lightview, float4(input.wposition.xyz, 1.0)));
        float angle = dot(plightspace, float3(0, 0, -1));
      //  return float4(angle, angle, angle, angle);
    

        float ta = saturate(angle - lightconeangle) / (1.0 - lightconeangle);

        float t = pow(dist / lightradius, 4);
        float att = saturate(pow(1 - t, 2)) / (dist * dist + 1);
        float3 ambient = float3(0.0005f, 0.0005, 0.0005) * albedo * float3(ao, ao, ao);

        float3 final = ambient + (diff + spec) * NL * lightcolor.xyz * att * lightintensity;
        final = final / (1 + final); // tone mapping
        final = pow(final, 1.0f / 2.2f);
        return float4(final, 1.0f);













    }
