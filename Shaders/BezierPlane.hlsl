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

struct VSOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 wposition : WPOSITION;
	uint id : ID;
};

struct DSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 wposition : WPOSITION;
    float2 patchpos : PATCHUV;
    float3 patches[16] : PATCHES;
    float3 test[2] : TEST;
    uint id : ID;
};




VSOutput VSMain(float3 position : POSITION, float3 normal : NORMAL, uint instanceid : SV_InstanceID)
{
	VSOutput result;

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
PatchTess ConstantHS(InputPatch<VSOutput, 16> patch, uint patchid : SV_PrimitiveID)
{
    float3 center = (patch[5].wposition + patch[6].wposition + patch[9].wposition + patch[10].wposition) / 4;
    float disqr = dot((center - eye), (center - eye));
    float tessfac = (1 - saturate(disqr / 200)) * 32+1.0; // add 1.0 means we still going to draw a quad even the distance is very long
  //  tessfac = 50;
    PatchTess res;
    res.innerfact[0] = tessfac;
    res.innerfact[1] = tessfac;
    

    center = (patch[5].wposition + patch[9].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 200)) * 32 + 1.0;
  //  tessfac = 50;
    res.edgefac[0] = tessfac;
  
    center = (patch[5].wposition + patch[6].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 200)) * 32 + 1.0;
   // tessfac = 50;
    res.edgefac[1] = tessfac;

    center = (patch[6].wposition + patch[10].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 200)) * 32 + 1.0;
//    tessfac = 50;
    res.edgefac[2] = tessfac;

    center = (patch[9].wposition + patch[10].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 200)) * 32 + 1.0;
  //  tessfac = 50;
    res.edgefac[3] = tessfac;

    return res;
}
[domain("quad")] // tesselatoin mode, tri,line or quad
[partitioning("fractional_even")] // new pont generate mode
[outputtopology("triangle_ccw")] // output topo, cw or ccw , line
[outputcontrolpoints(16)] // decide how manny times that HS will evocke per patch, SV_ControlPointID is from 0~15, cause we only generate 16 contorl points
[patchconstantfunc("ConstantHS")] // define the function we want to use 
[maxtessfactor(32.0f)]
VSOutput HSMain(InputPatch<VSOutput, 16> patch,uint cid : SV_OutputControlPointID,uint pid : SV_PrimitiveID) // computation for each control points, can gemnerate conrol points;
{
    VSOutput controlpoint;
    controlpoint = patch[cid];
    return controlpoint;
}
float rand_1_05(float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}

float4 BernsteinBasis(float t)
{
    float invT = 1.0 - t;
    return float4(invT * invT * invT, 3.0 * t * invT * invT, 3.0 * t * t * invT, t * t * t);
}
float4 DBernsteinBasis(float t)
{
    float invT = saturate(1.0f - t);
    return float4(-3.0f * invT * invT, 3.0 * invT * invT - 6.0f * t * invT, 6.0f * t * invT - 3.0f*t*t, 3.0f*t * t );
}
float contiBezier(float v1, float v2, float v3, float v4,float4 basis)
{
    float points[4];
    points[0] = v2;
    points[1] = v2 + (v3 - v1)*0.5f;
    points[2] = v3 + (v2 - v4)*0.5f;
    points[3] = v3;
    float3 res = 0;
    res = basis.x * points[0] + basis.y * points[1] + basis.z * points[2] + basis.w * points[3];
    return res;
}

[domain("quad")]
DSOutput DSMain(PatchTess fac, float2 uv : SV_DomainLocation, const OutputPatch<VSOutput, 16> patch)
{
    DSOutput outpoint;
    float3 pos1 = lerp(patch[5].wposition, patch[6].wposition, uv.x); // caculate position
    float3 pos2 = lerp(patch[9].wposition, patch[10].wposition,uv.x);
    outpoint.wposition = lerp(pos1, pos2, uv.y);

    float4 basis = BernsteinBasis(uv.x);
    float curve1 = contiBezier(patch[0].wposition.y, patch[1].wposition.y, patch[2].wposition.y, patch[3].wposition.y, basis);
    float curve2 = contiBezier(patch[4].wposition.y, patch[5].wposition.y, patch[6].wposition.y, patch[7].wposition.y, basis);
    float curve3 = contiBezier(patch[8].wposition.y, patch[9].wposition.y, patch[10].wposition.y, patch[11].wposition.y, basis);
    float curve4 = contiBezier(patch[12].wposition.y, patch[13].wposition.y, patch[14].wposition.y, patch[15].wposition.y, basis);
   // outpoint.wposition.y = rand_1_05(uv);
    basis = BernsteinBasis(uv.y);
    outpoint.wposition.y = contiBezier(curve1, curve2, curve3, curve4, basis);


    basis = BernsteinBasis(uv.x * 0.999);
    curve1 = contiBezier(patch[0].wposition.y, patch[1].wposition.y, patch[2].wposition.y, patch[3].wposition.y, basis);
    curve2 = contiBezier(patch[4].wposition.y, patch[5].wposition.y, patch[6].wposition.y, patch[7].wposition.y, basis);
    curve3 = contiBezier(patch[8].wposition.y, patch[9].wposition.y, patch[10].wposition.y, patch[11].wposition.y, basis);
    curve4 = contiBezier(patch[12].wposition.y, patch[13].wposition.y, patch[14].wposition.y, patch[15].wposition.y, basis);
    basis = DBernsteinBasis(uv.y * 0.999);
    outpoint.normal.z = contiBezier(curve1, curve2, curve3, curve4, basis);
    float3 xvec = float3(0.0, outpoint.normal.z, 1.0);


    basis = BernsteinBasis(uv.y * 0.999);
    curve1 = contiBezier(patch[0].wposition.y, patch[4].wposition.y, patch[8].wposition.y, patch[12].wposition.y, basis);
    curve2 = contiBezier(patch[1].wposition.y, patch[5].wposition.y, patch[9].wposition.y, patch[13].wposition.y, basis);
    curve3 = contiBezier(patch[2].wposition.y, patch[6].wposition.y, patch[10].wposition.y, patch[14].wposition.y, basis);
    curve4 = contiBezier(patch[3].wposition.y, patch[7].wposition.y, patch[11].wposition.y, patch[15].wposition.y, basis);
    basis = DBernsteinBasis(uv.x * 0.999);
    outpoint.normal.x = contiBezier(curve1, curve2, curve3, curve4, basis);
    float3 zvec = float3(1.0, outpoint.normal.x, 0.0);

    outpoint.normal.y = 1;

    outpoint.normal = normalize(cross(xvec, zvec));


   // [unroll]
  
    outpoint.patches[0] = patch[0].wposition;
    outpoint.patches[1] = patch[1].wposition;
    outpoint.patches[2] = patch[2].wposition;
    outpoint.patches[3] = patch[3].wposition;
    outpoint.patches[4] = patch[4].wposition;
    outpoint.patches[5] = patch[5].wposition;
    outpoint.patches[6] = patch[6].wposition;
    outpoint.patches[7] = patch[7].wposition;
    outpoint.patches[8] = patch[8].wposition;
    outpoint.patches[9] = patch[9].wposition;
    outpoint.patches[10] = patch[10].wposition;
    outpoint.patches[11] = patch[11].wposition;
    outpoint.patches[12] = patch[12].wposition;
    outpoint.patches[13] = patch[13].wposition;
    outpoint.patches[14] = patch[14].wposition;
    outpoint.patches[15] = patch[15].wposition;



    outpoint.patchpos = uv;


  //  if (outpoint.normal.y<0)
  //      outpoint.normal = -outpoint.normal;

   // outpoint.wposition = curve1;

   // outpoint.wposition.y = cos(uv.x*20) + sin(uv.y*20);

    outpoint.position = float4(outpoint.wposition, 1.0f);
    outpoint.position = mul(view, outpoint.position);
    outpoint.position = mul(proj, outpoint.position);
    
 //   outpoint.normal = patch[0].normal;
    outpoint.id = patch[0].id;

    return outpoint;

}





float4 PSMain(DSOutput input) : SV_TARGET
{

    float3 normal;

    // calcuate normal in pixel rate can largely smooth the surface, however, the un-accept result for normal could be cause by the wrong vector at the edge

    float4 basis = BernsteinBasis(input.patchpos.x);
    float curve1 = contiBezier(input.patches[0].y, input.patches[1].y, input.patches[2].y, input.patches[3].y, basis);
    float curve2 = contiBezier(input.patches[4].y, input.patches[5].y, input.patches[6].y, input.patches[7].y, basis);
    float curve3 = contiBezier(input.patches[8].y, input.patches[9].y, input.patches[10].y, input.patches[11].y, basis);
    float curve4 = contiBezier(input.patches[12].y, input.patches[13].y, input.patches[14].y, input.patches[15].y, basis);
    basis = DBernsteinBasis(input.patchpos.y);
    normal.z = contiBezier(curve1, curve2, curve3, curve4, basis);
    float3 tangent = float3(0.0, normal.z, 1.0);


    basis = BernsteinBasis(input.patchpos.y);
    curve1 = contiBezier(input.patches[0].y, input.patches[4].y, input.patches[8].y, input.patches[12].y, basis);
    curve2 = contiBezier(input.patches[1].y, input.patches[5].y, input.patches[9].y, input.patches[13].y, basis);
    curve3 = contiBezier(input.patches[2].y, input.patches[6].y, input.patches[10].y, input.patches[14].y, basis);
    curve4 = contiBezier(input.patches[3].y, input.patches[7].y, input.patches[11].y, input.patches[15].y, basis);
    basis = DBernsteinBasis(input.patchpos.x);
    normal.x = contiBezier(curve1, curve2, curve3, curve4, basis);
    float3 bitangent = float3(1.0, normal.x, 0.0);

    normal.y = 0;
    normal = normalize(cross(tangent, bitangent));
    //if (input.patchpos.x <= 0.3 || input.patchpos.y <= 0.3f || input.patchpos.x >= 0.7 || input.patchpos.y >= 0.7f)
    //    return float4(normal.x, normal.y, normal.z, 1.0);


    //discard;
 //   if (normal.y<0)
 //       normal = normal;
//    normal = normalize(normal);






   // float3 normal = normalize(input.normal);
//	float3 lightColor = float3(1.0,1.0,1.0);
 //   return float4(normal.x, normal.y, normal.z, 1.0);

 //   return float4(normal.x * 0.5 + 0.5, 0, normal.z * 0.5 + 0.5, 1.0);
  //  return float4(input.wposition.x, 0.0, input.wposition.z, 1.0) * input.wposition.y;
    return float4(0.2,1.0, 1.0, 1.0) * input.wposition.y;

}
