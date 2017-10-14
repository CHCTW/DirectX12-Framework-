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
cbuffer Time : register(b2)
{
    float time;
};

    StructuredBuffer<InstancedInformation> instances: register(t0);

struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 wposition : WPOSITION;
    float scale : SCALE;
	uint id : ID;
};
struct GSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 wposition : WPOSITION;
    uint id : ID;
};


GSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, uint instanceid : SV_InstanceID)
{
    GSInput result;

//	result.position = mul(instances[instanceid].model, float4(position, 1.0f));

    result.position = float4(position, 1.0f);

	result.wposition = result.position.xyz;
//	result.position = mul(view, result.position);
//	result.position = mul(proj,result.position);
    result.normal = normal;
//	result.normal = mul(instances[instanceid].normal, float4(normal, 0.0f)).xyz;
	result.id = instanceid;
	return result;
}



void GenerateFurTrainges(GSInput base[3],out PSInput fur[4],float scale)
{
    float4 vec1 = base[0].position - base[1].position;
    float4 vec2 = base[0].position - base[2].position;

    float4 center = (base[0].position + base[1].position + base[2].position) / 3;
    float3 sharpnormal = cross(vec1.xyz, vec2.xyz);
    
    
    fur[0].position = float4(sharpnormal, 0.0) / (0.3+scale) * 10 + center;
    fur[0].normal = sharpnormal;
    fur[0].id = base[0].id;

    //fur[1] = base[0];
    //fur[2] = base[1];
    //fur[3] = base[2];

   
    fur[1].position = (base[0].position + base[1].position)/2;
    fur[1].normal = fur[1].position.xyz - center.xyz;
    fur[1].id = base[0].id;


    fur[2].position = (base[1].position + base[2].position) / 2;
    fur[2].normal = fur[2].position.xyz - center.xyz;
    fur[2].id = base[0].id;


    fur[3].position = (base[2].position + base[0].position) / 2;
    fur[3].normal = fur[3].position.xyz - center.xyz;
    fur[3].id = base[0].id;





     //   float3 n = base[j].normal;
     //   fur[j + 1].normal = n;
   





 //   fur[1].position = (base[0].position + base[1].position)/2;
 //   fur[1].normal = fur[1].position - center;
 //   fur[1].id = base[1].id;

   
}
[maxvertexcount(10)]
void GSMain(triangle GSInput gin[3], inout TriangleStream<PSInput> stream)
{

    PSInput fur[4];

   

    float4 vec1 = gin[0].position - gin[1].position;
    float4 vec2 = gin[0].position - gin[2].position;

    float4 center = (gin[0].position + gin[1].position + gin[2].position)/3;

    float3 movedir = center.xyz;
    float y = movedir.y;
   movedir.y = 0;
    movedir = normalize(movedir);
    float scale = cos((y + time / 3) * 5) + 1.0;
    movedir *= (scale / 3);
    
 //   movedir = float3(0, 0, 0);
    GenerateFurTrainges(gin, fur,scale);

    float3 sharpnormal = cross(vec1.xyz, vec2.xyz);
    PSInput gout;

    for (int i = 0; i < 3; ++i)
    {
 
        gout.position = gin[i].position;
        gout.normal = gin[i].normal;
        gout.id = gin[i].id;
      

        //gout.normal = sharpnormal;
        gout.position = (gin[i].position - center) + center;

        gout.position = mul(instances[gout.id].model, gout.position + float4(movedir, 0));
        gout.wposition = gout.position.xyz / gout.position.w;



        gout.position = mul(view, gout.position);
        gout.position = mul(proj, gout.position);
        gout.normal = mul(instances[gout.id].normal, float4(gin[i].normal.xyz, 0.0f)).xyz;
        gout.scale = scale;
        stream.Append(gout);
    }
    stream.RestartStrip();


        [unroll]
    for (int i = 0; i < 4; ++i)
    {
    
        fur[i].position = mul(instances[fur[i].id].model, fur[i].position + float4(movedir, 0));
        fur[i].wposition = fur[i].position.xyz / fur[i].position.w;
        fur[i].normal = mul(instances[fur[i].id].normal, float4(fur[i].normal, 0.0));
        fur[i].position = mul(view, fur[i].position);
        fur[i].position = mul(proj, fur[i].position);

        fur[i].scale = scale;
    }




    stream.Append(fur[3]);
    stream.Append(fur[0]);
    stream.Append(fur[2]);
    stream.Append(fur[1]);

    stream.RestartStrip();


    
    stream.Append(fur[1]);
    stream.Append(fur[0]);
    stream.Append(fur[3]);



}

float3 Fresnel(float3 F0,float HV)
{
    return F0 + (1.0 - F0) * pow(2, (-5.55473 * HV - 6.98316) * HV);
//	return F0 + (1.0 - F0)*pow(1 - HV, 5.0f);
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

//	float3 lightColor = float3(1.0,1.0,1.0);
	
	
	float roughness = instances[input.id].roughness;
	float metallic = instances[input.id].metallic;
    float3 albedo = pow(instances[input.id].albedo, 2.2);
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
   // return float4(F.y, F.y, F.y,1.0);
   // return float4(N, 1.0);

    
//    return float4((normalize(input.wposition) * 0.5 + 0.5) * input.scale, 1.0);

    return float4(input.wposition.y*N.z, input.scale, sqrt(1.0 - input.scale)*N.z, 1.0);

	float3 spec = D*F*G/(4 * NV * NL + 0.001);


	float3 Kd = 1 - F;  // diffuse color 
	Kd = Kd * (1.0 - metallic);
	float3 diff = Kd*albedo / PI;

	float dist = length(lightpostion.xyz - input.wposition);
	//float att = 1.0f / (1 + lightattenuation.y*dist + dist*dist*lightattenuation.z);


    float t = pow(dist / lightradius, 4);
    float att = saturate(pow(1 - t, 2)) / (dist * dist + 1);

	if (dist > lightradius)
		att = 0;
	float3 ambient =  float3(0.05f,0.05,0.05)*albedo * float3(ao,ao,ao);

	float3 final = ambient + (diff +spec)*NL*lightcolor.xyz*att;
	final = final / (1 + final); // tone mapping
	final = pow(final, 1.0f / 2.2f);
	return float4(final,1.0f);
}
