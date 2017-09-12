#define PI 3.14159
struct InstancedInformation
{
	float4x4 model;
	float4x4 normal;
	float roughness;
	float metallic;
	float3 albedo;
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
	float4 lightattenuation;
	float lightradius;
};
StructuredBuffer<InstancedInformation> instances: register(t0);
Texture2D ShadowMap : register(t1);
SamplerState Sampler : register(s0);
struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 wposition : WPOSITION;
	uint id : ID;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, uint instanceid : SV_InstanceID)
{
	PSInput result;

	result.position = mul(instances[instanceid].model, float4(position, 1.0f));
	result.wposition = result.position.xyz;
	result.position = mul(view, result.position);
	result.position = mul(proj,result.position);
	result.normal = mul(instances[instanceid].normal, float4(normal, 0.0f)).xyz;
	result.id = instanceid;
	return result;
}
float3 Fresnel(float3 F0,float costheta)
{
	return F0 + (1.0 - F0)*pow(1 - costheta, 5.0f);
}


float Distribution(float3 N, float3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}
float GeometrySchlickGGX(float NV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

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

float ShadowTest(float3 pos)
{
	float4 shadowcood = mul(lightview, float4(pos, 1.0));
	shadowcood = mul(lightproj, shadowcood);
	
	shadowcood.xyz = shadowcood.xyz/shadowcood.w;
	float pixdepth = shadowcood.z;
	shadowcood.xy = shadowcood.xy*0.5 + 0.5;
	shadowcood.y = 1 - shadowcood.y;

	float dis = dot(shadowcood - float2(0.5, 0.5), shadowcood - float2(0.5, 0.5));

	if (shadowcood.w >= 0)
	{
	//	if (shadowcood.x > 0 && shadowcood.x < 1)
		//	if (shadowcood.y > 0 && shadowcood.y  < 1)
			if(dis<=0.25)
			{
				if (pixdepth > ShadowMap.Sample(Sampler, shadowcood.xy).r+0.0001f)
					return 1.0f;
				//return ShadowMap.Sample(Sampler, shadowcood.xy / shadowcood.w*0.5f + 0.5f);
				return 0.0f;
			}
	}
	return 1.0f;
}
float4 PSMain(PSInput input) : SV_TARGET
{

//	float3 lightColor = float3(1.0,1.0,1.0);
	
	
	float roughness = instances[input.id].roughness;
	float metallic = instances[input.id].metallic;
	float3 albedo = instances[input.id].albedo;
	float ao = 1.0f;


	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F

	float3 N = normalize(input.normal);  
	float3 V = normalize(eye-input.wposition);
	float3 L = normalize(lightpostion.xyz - input.wposition);
	float3 H = normalize(L + V);
	float LH = max(dot(L, H), 0.0f);
	float NL = max(dot(L, N), 0.0f);
	float NV = max(dot(N, V), 0.0f);
	float HV = max(dot(H, V), 0.0f);

	float3 F = Fresnel(F0, HV);
	float D = Distribution(N, H, roughness);
	float3 G = GeometrySmith(NV, NL, roughness);

	float3 spec = D*F*G/(4 * NV * NL + 0.001);


	float3 Kd = 1 - F;  // diffuse color 
	Kd = Kd * (1.0 - metallic);
	float3 diff = Kd*albedo / PI;

	float dist = length(lightpostion.xyz - input.wposition);
	float att = 1.0f / (1 + lightattenuation.y*dist + dist*dist*lightattenuation.z);
//	if (dist > lightradius*2)
//		att = 0;
	float3 ambient =  float3(0.0005f,0.0005,0.0005)*albedo * float3(ao,ao,ao);

	float test = ShadowTest(input.wposition);

	float3 final = ambient + (diff + spec)*NL*lightcolor.xyz*att*(1-test);
	final = final / (1 + final); // tone mapping
	final = pow(final, 1.0f / 2.2f);


	

	return float4(final,1.0f);
}
