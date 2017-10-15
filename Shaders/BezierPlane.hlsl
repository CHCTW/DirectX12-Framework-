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
struct PatchTess
{
    float edgefac[4] : SV_TessFactor;
    float innerfact[2] : SV_InsideTessFactor;
};
PatchTess ConstantHS(InputPatch<PSInput, 4> patch, uint patchid : SV_PrimitiveID)
{
    float3 center = (patch[0].wposition + patch[1].wposition + patch[2].wposition + patch[3].wposition) / 4;
    float disqr = dot((center - eye), (center - eye));
    float tessfac = (1 - saturate(disqr / 300)) * 64+10.0; // add 1.0 means we still going to draw a quad even the distance is very long
    PatchTess res;
    res.edgefac[0] = tessfac;
    res.edgefac[1] = tessfac;
    res.edgefac[2] = tessfac;
    res.edgefac[3] = tessfac;
    res.innerfact[0] = tessfac;
    res.innerfact[1] = tessfac;
    return res;
}
[domain("quad")] // tesselatoin mode, tri,line or quad
[partitioning("integer")] // new pont generate mode
[outputtopology("triangle_ccw")] // output topo, cw or ccw , line
[outputcontrolpoints(4)] // decide how manny times that HS will evocke per patch, SV_ControlPointID is from 0~3, cause we only generate 4 contorl points
[patchconstantfunc("ConstantHS")] // define the function we want to use 
[maxtessfactor(64.0f)]
PSInput HSMain(InputPatch<PSInput, 4> patch,uint cid : SV_OutputControlPointID,uint pid : SV_PrimitiveID) // computation for each control points, can gemnerate conrol points;
{
    PSInput controlpoint;
    controlpoint = patch[cid];
    return controlpoint;
}
float rand_1_05(float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}
[domain("quad")]
PSInput DSMain(PatchTess fac,float2 uv : SV_DomainLocation,const OutputPatch<PSInput,4> patch)
{
    PSInput outpoint;
    float3 pos1 = lerp(patch[0].wposition, patch[1].wposition, uv.x); // caculate position
    float3 pos2 = lerp(patch[2].wposition, patch[3].wposition, uv.x);
    outpoint.wposition = lerp(pos1, pos2, uv.y);


    outpoint.wposition.y = rand_1_05(uv);
    
    outpoint.wposition.y = cos(uv.x*10) + cos(uv.y*10);

    outpoint.position = float4(outpoint.wposition, 1.0f);
    outpoint.position = mul(view, outpoint.position);
    outpoint.position = mul(proj, outpoint.position);
    
    outpoint.normal = patch[0].normal;
    outpoint.id = patch[0].id;

    return outpoint;

}





float4 PSMain(PSInput input) : SV_TARGET
{

//	float3 lightColor = float3(1.0,1.0,1.0);
	
    return float4(input.wposition*0.5+0.5, 1.0);
    return float4(0.2, 1.0, 1.0, 1.0);

}
