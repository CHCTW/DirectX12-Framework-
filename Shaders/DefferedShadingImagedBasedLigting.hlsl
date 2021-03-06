#define PI 3.14159
cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float3 eye;
	float padding;
	float4x4 viewinverse;
	float4x4 projinverse;
};
struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
Texture2D NorMettexture : register(t0);
Texture2D AlbRoutexture : register(t1);
Texture2D Depthtexture : register(t2);

TextureCube Irradiance: register(t4);
TextureCube Specular : register(t5);
Texture2D BRDFIntergrate : register(t6);

SamplerState g_sampler : register(s0);
PSInput VSMain(uint id : SV_VertexID)
{
	PSInput result;
	result.uv = float2((id << 1) & 2, id & 2);
	result.position = float4(result.uv  * float2(2, -2) + float2(-1, 1), 0, 1);
	return result;
}
float3 Fresnel(float3 F0, float costheta)
{
    //return F0 + (1.0 - F0) * pow(2, (-5.55473 * costheta - 6.98316) * costheta); // unreal4's function largely 
	return F0 + (1.0 - F0)*pow(1 - costheta, 5.0f); // in metal the frensel will become super white when NV close to 1
                                                    // but it probally only show up when NV really close to 1
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
float4 PSMain(PSInput input) : SV_TARGET
{
	//return float4(input.uv,0.0f,0.0f);
	float3 normal = NorMettexture.Sample(g_sampler, input.uv).rgb;
	float metallic = NorMettexture.Sample(g_sampler, input.uv).a;
	float3 albedo = AlbRoutexture.Sample(g_sampler, input.uv).rgb;
	float roughness = AlbRoutexture.Sample(g_sampler, input.uv).a;
	float depth = Depthtexture.Sample(g_sampler, input.uv).r;
	float4 projcoord;
	projcoord .xy = input.uv;
	projcoord.y = 1.0 - projcoord.y;
	projcoord.xy = projcoord.xy*2.0f - 1.0f;
	projcoord.z = depth;
	projcoord.w = 1.0f;
	float4 pos = mul(projinverse,projcoord);
//	pos.xyz = pos.xyz / pos.w;
	pos = mul(viewinverse, pos);
	pos.xyz = pos.xyz / pos.w;

//	return pos;



//	lightpostion = mul(view, lightpostion);

//	lightpostion.xyz = lightpostion.xyz / lightpostion.w;

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F

   // return float4(F0.z, F0.z, F0.z, 1.0);


	float3 N = normalize(normal);
	float3 V = normalize(eye-pos.xyz);


//	float HV = max(dot(H, V), 0.0f);
    float NV = saturate(dot(N, V));
    float3 R = 2 * dot(V, N) * N - V;


  //  if (NV<0.1)
   //     return float4(1.0, 1.0, 0.0, 0.0);
//	return float4(pos.xyz, 1.0);

    float3 F = F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1 - NV, 5.0f);
 //  float3 F = Fresnel(F0, NV);
   // return float4(F.z, F.z, F.z, 1.0);


    float3 Ks = F; // diffuse color 
	float3 Kd = 1-Ks;

    Kd *= 1.0 - metallic;  // metalic will absorb all refreaction and will never diffuse
   

    float3 prefilcolor = Specular.SampleLevel(g_sampler, R, roughness *5.0f).rgb;
    float2 BRDF = BRDFIntergrate.Sample(g_sampler, float2(NV, roughness)).rg;
    float3 specular = prefilcolor * (Ks * BRDF.x + BRDF.y);
    float3 diff = albedo * Irradiance.Sample(g_sampler, R).rgb;
	float3 ambient = Kd*diff+specular;

	float3 final = ambient;
    final = final / (float3(1.0, 1.0, 1.0) + final); // tone mapping
    final = pow(final, 1/2.2f);
//	pos.xyz = pos.xyz / 100;
//	pos.xyz = normalize(pos.xyz);
	return float4(final,1.0);
}
