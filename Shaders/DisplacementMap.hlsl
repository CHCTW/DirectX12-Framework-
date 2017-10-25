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

struct DSOutput
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

	result.position = mul(instances[instanceid].model, float4(position, 1.0f));
	result.wposition = result.position.xyz;

 //   result.wposition = result.wposition + blockbase[2].SampleLevel(loopsample, uv,0).xyz;

	result.position = mul(view, result.position);
	result.position = mul(proj,result.position);
	
    result.uv = uv;
    result.tangent = mul(instances[instanceid].normal, float4(tangent, 0.0f)).xyz;
	result.normal = mul(instances[instanceid].normal, float4(normal, 0.0f)).xyz;
	result.id = instanceid;
	return result;
}
struct PatchTess
{
    float edgefac[3] : SV_TessFactor;
    float innerfact : SV_InsideTessFactor;
};
PatchTess ConstantHS(InputPatch<VSOutput, 3> patch, uint patchid : SV_PrimitiveID)
{
    float3 center = (patch[0].wposition + patch[1].wposition + patch[2].wposition ) / 3;
    float disqr = dot((center - eye), (center - eye));
    float tessfac = (1 - saturate(disqr / 500)) * 64+1.0; // add 1.0 means we still going to draw a quad even the distance is very long
    float3 vec1 = patch[0].wposition - patch[1].wposition;
    float3 vec2 = patch[0].wposition - patch[2].wposition;

    PatchTess res;




    
    res.innerfact = tessfac;

    

    center = (patch[0].wposition + patch[1].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 500)) * 64 + 1.0;
    res.edgefac[2] = tessfac;
  
    center = (patch[1].wposition + patch[2].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 500)) * 64 + 1.0;
    res.edgefac[0] = tessfac;

    center = (patch[2].wposition + patch[0].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 500)) * 64 + 1.0;
    res.edgefac[1] = tessfac;



    return res;
}
[domain("tri")] // tesselatoin mode, tri,line or quad
[partitioning("fractional_even")] // new pont generate mode
[outputtopology("triangle_ccw")] // output topo, cw or ccw , line
[outputcontrolpoints(3)] // decide how manny times that HS will evocke per patch, SV_ControlPointID is from 0~3, cause we only generate 16 contorl points
[patchconstantfunc("ConstantHS")] // define the function we want to use 
[maxtessfactor(32.0f)]
VSOutput HSMain(InputPatch<VSOutput, 3> patch,uint cid : SV_OutputControlPointID,uint pid : SV_PrimitiveID) // computation for each control points, can gemnerate conrol points;
{
    VSOutput controlpoint;
    controlpoint = patch[cid];
    return controlpoint;
}


[domain("tri")]
DSOutput DSMain(PatchTess fac, float3 uvw : SV_DomainLocation, const OutputPatch<VSOutput, 3> patch)
{
    DSOutput outpoint;

   

    outpoint.normal = patch[0].normal * uvw.x + patch[1].normal * uvw.y + patch[2].normal * uvw.z;
    outpoint.tangent = patch[0].tangent * uvw.x + patch[1].tangent * uvw.y + patch[2].tangent * uvw.z;
    

    outpoint.uv = patch[0].uv * uvw.x + patch[1].uv * uvw.y + patch[2].uv * uvw.z;
    float heightscale = 3;
   float h = blockbase[2].SampleLevel(loopsample, outpoint.uv,0).r;

    outpoint.wposition = patch[0].wposition * uvw.x + patch[1].wposition * uvw.y + patch[2].wposition * uvw.z;
    outpoint.wposition.xyz += (normalize(outpoint.normal) * h * height);

    outpoint.position = float4(outpoint.wposition, 1.0f);
    outpoint.position = mul(view, outpoint.position);
    outpoint.position = mul(proj, outpoint.position);
    outpoint.id = patch[0].id;

    return outpoint;

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

float4 PSMain(DSOutput input) : SV_TARGET
{


    float3 normaltext = blockbase[1].SampleLevel(loopsample, input.uv,0)*2.0-1.0;
    float3 normalobject = normalize(input.normal);
    float3 tan = normalize(input.tangent);
    float3 bitan = cross(tan,normalobject);




    float3 normal = normaltext.x * tan + normaltext.y * bitan + normaltext.z * normalobject;

    float roughness = blockbase[3].Sample(loopsample, input.uv).r;
    float metallic = 0.0;


    float3 albedo = pow(blockbase[0].Sample(loopsample, input.uv), 2.2);
    float ao = 1.0f;


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


    float t = pow(dist / lightradius, 4);
    float att = saturate(pow(1 - t, 2)) / (dist * dist + 1);

    if (dist > lightradius)
        att = 0;
    float3 ambient = float3(0.0005f, 0.0005, 0.0005) * albedo * float3(ao, ao, ao);

    float3 final = ambient + (diff + spec) * NL * lightcolor.xyz * att;
    final = final / (1 + final); // tone mapping
    final = pow(final, 1.0f / 2.2f);
    return float4(final, 1.0f);













}
