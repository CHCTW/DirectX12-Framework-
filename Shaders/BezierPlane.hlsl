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
    float lightradius;
    float lightintensity;
    float lightconeangle;
    float padding;
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
    PatchTess res;
    res.innerfact[0] = tessfac;
    res.innerfact[1] = tessfac;
    

    center = (patch[5].wposition + patch[9].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 200)) * 32 + 1.0;
    res.edgefac[0] = tessfac;
  
    center = (patch[5].wposition + patch[6].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 200)) * 32 + 1.0;

    res.edgefac[1] = tessfac;

    center = (patch[6].wposition + patch[10].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 200)) * 32 + 1.0;

    res.edgefac[2] = tessfac;

    center = (patch[9].wposition + patch[10].wposition) / 2;
    disqr = dot((center - eye), (center - eye));
    tessfac = (1 - saturate(disqr / 200)) * 32 + 1.0;

    res.edgefac[3] = tessfac;

    return res;
}
[domain("quad")] // tesselatoin mode, tri,line or quad
[partitioning("integer")] // new pont generate mode
[outputtopology("triangle_ccw")] // output topo, cw or ccw , line
[outputcontrolpoints(16)] // decide how manny times that HS will evocke per patch, SV_ControlPointID is from 0~15, cause we only generate 16 contorl points
[patchconstantfunc("ConstantHS")] // define the function we want to use 
[maxtessfactor(64.0f)]
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
    return float4(invT * invT * invT, 3.0 * t * invT * invT, 3.0f * t * t * invT, t * t * t);
}
float4 DBernsteinBasis(float t)
{
    float invT = 1.0f - t;
    if(t==0.0f)
        return float4(-3.0f, 3.0f, 0.0, 0.0);
    if(t==1.0f)
        return float4(0.0, 0.0, -3.0, 3.0);
    return float4(-3.0f * invT * invT, 3.0 * invT * invT - 6.0f * t * invT, 6.0f * t * invT - 3.0f * t * t, 3.0f * t * t);
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



float3 BezierCurve(in const OutputPatch<VSOutput, 16> patch,float2 uv)
{
    float pos;
    float pos1 = lerp(patch[5].wposition.y, patch[6].wposition.y, uv.x); // caculate position
    float pos2 = lerp(patch[9].wposition.y, patch[10].wposition.y, uv.x);
    pos = lerp(pos1, pos2, uv.y);
    float4 basis = BernsteinBasis(uv.x);
    float curve1 = contiBezier(patch[0].wposition.y, patch[1].wposition.y, patch[2].wposition.y, patch[3].wposition.y, basis);
    float curve2 = contiBezier(patch[4].wposition.y, patch[5].wposition.y, patch[6].wposition.y, patch[7].wposition.y, basis);
    float curve3 = contiBezier(patch[8].wposition.y, patch[9].wposition.y, patch[10].wposition.y, patch[11].wposition.y, basis);
    float curve4 = contiBezier(patch[12].wposition.y, patch[13].wposition.y, patch[14].wposition.y, patch[15].wposition.y, basis);
    basis = BernsteinBasis(uv.y);
    pos = contiBezier(curve1, curve2, curve3, curve4, basis);
    return pos;
}


float3 BezierCurveP(in const float3  patch[16], float2 uv)
{
    float pos;
    float pos1 = lerp(patch[5].y, patch[6].y, uv.x); // caculate position
    float pos2 = lerp(patch[9].y, patch[10].y, uv.x);
    pos = lerp(pos1, pos2, uv.y);
    float4 basis = BernsteinBasis(uv.x);
    float curve1 = contiBezier(patch[0].y, patch[1].y, patch[2].y, patch[3].y, basis);
    float curve2 = contiBezier(patch[4].y, patch[5].y, patch[6].y, patch[7].y, basis);
    float curve3 = contiBezier(patch[8].y, patch[9].y, patch[10].y, patch[11].y, basis);
    float curve4 = contiBezier(patch[12].y, patch[13].y, patch[14].y, patch[15].y, basis);
   // outpoint.wposition.y = rand_1_05(uv);
    basis = BernsteinBasis(uv.y);
    pos = contiBezier(curve1, curve2, curve3, curve4, basis);
    return pos;
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
    basis = BernsteinBasis(uv.y);
    outpoint.wposition.y = contiBezier(curve1, curve2, curve3, curve4, basis);


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




    outpoint.position = float4(outpoint.wposition, 1.0f);
    outpoint.position = mul(view, outpoint.position);
    outpoint.position = mul(proj, outpoint.position);
    
 //   outpoint.normal = patch[0].normal;
    outpoint.id = patch[0].id;

    return outpoint;

}
[maxvertexcount(9)]
void GSMain(triangle DSOutput inpoints[3],inout LineStream<DSOutput> stream)
{
    stream.Append(inpoints[0]);
    stream.Append(inpoints[1]);
    stream.Append(inpoints[2]);
    stream.Append(inpoints[0]);


    stream.RestartStrip();
    DSOutput base;
    DSOutput extend;
    for (int i = 0; i < 3; ++i)
    {
        base = inpoints[i];
        extend = base;
        extend.normal = base.normal;

        extend.wposition = base.wposition + normalize(base.normal)*3;
        extend.position = float4(extend.wposition, 1.0f);
        extend.position = mul(view, extend.position);
        extend.position = mul(proj, extend.position);


        stream.Append(base);
        stream.Append(extend);
        stream.RestartStrip();
    }

}


float3 Fresnel(float3 F0, float HV)
{
    return F0 + (1.0 - F0) * pow(2, (-5.55473 * HV - 6.98316) * HV);
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
 //   return float4(1.0,1.0,0.0, 1.0);

    float3 normal = normalize(input.normal);

    // calcuate normal in pixel rate can largely smooth the surface, however, the un-accept result for normal could be cause by the wrong vector at the edge
  

    float4 basis = BernsteinBasis(input.patchpos.x);
    float curve1 = contiBezier(input.patches[0].y, input.patches[1].y, input.patches[2].y, input.patches[3].y, basis);
    float curve2 = contiBezier(input.patches[4].y, input.patches[5].y, input.patches[6].y, input.patches[7].y, basis);
    float curve3 = contiBezier(input.patches[8].y, input.patches[9].y, input.patches[10].y, input.patches[11].y, basis);
    float curve4 = contiBezier(input.patches[12].y, input.patches[13].y, input.patches[14].y, input.patches[15].y, basis);
    basis = DBernsteinBasis(input.patchpos.y * 0.99 + 0.001);
    normal.z = contiBezier(curve1, curve2, curve3, curve4, basis);
    float3 temp = input.patches[6] - input.patches[10];
    float3 tangent = temp;
    tangent.y = -normal.z;


    basis = BernsteinBasis(input.patchpos.y);
    curve1 = contiBezier(input.patches[0].y, input.patches[4].y, input.patches[8].y, input.patches[12].y, basis);
    curve2 = contiBezier(input.patches[1].y, input.patches[5].y, input.patches[9].y, input.patches[13].y, basis);
    curve3 = contiBezier(input.patches[2].y, input.patches[6].y, input.patches[10].y, input.patches[14].y, basis);
    curve4 = contiBezier(input.patches[3].y, input.patches[7].y, input.patches[11].y, input.patches[15].y, basis);
    basis = DBernsteinBasis(input.patchpos.x * 0.99 + 0.01);
    normal.x = contiBezier(curve1, curve2, curve3, curve4, basis);
  //  temp = 
    temp = (input.patches[9] - input.patches[10]);

    float3 bitangent = temp;
    bitangent.y = -normal.x;



    normal = normalize(cross(normalize(tangent), normalize(bitangent)));
  //  return float4(normal, 1.0);

    float roughness = 0.1;
    float metallic = 0.9;
    float3 albedo = pow(normalize(input.wposition)*0.5+0.5, 2.2);
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
    float3 plightspace = normalize(mul(lightview, float4(input.wposition.xyz, 1.0)));
    float angle = dot(plightspace, float3(0, 0, -1));
  //  return float4(angle, angle, angle, angle);
    

    float ta = saturate(angle - lightconeangle) / (1.0 - lightconeangle);

    float t = pow(dist / lightradius, 4);
    float att = saturate(pow(1 - t, 2)) / (dist * dist + 1) * ta;

    if (dist > lightradius)
        att = 0;
    float3 ambient = float3(0.0005f, 0.0005, 0.0005) * albedo * float3(ao, ao, ao);

    float3 final = ambient + (diff + spec) * NL * lightcolor.xyz * att*lightintensity;
    final = final / (1 + final); // tone mapping
    final = pow(final, 1.0f / 2.2f);
    return float4(final, 1.0f);













}
