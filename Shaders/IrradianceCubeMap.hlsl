
#define PI  3.14159265359
cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 view;
    float4x4 proj;
    float3 eye;
    float padding;
    float4x4 viewinverse;
    float4x4 projinverse;
};
cbuffer FaceConst : register(b1)
{
    uint face;
};
TextureCube cube : register(t0);
SamplerState g_sampler : register(s0);
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL)
{
    PSInput result;
    result.normal = -position.xyz;
    result.position = mul(view, float4(position*80000 , 1.0f));
    result.position = mul(proj, result.position);
    result.position.z = result.position.w;
//	result.normal = normal;

    return result;
}


struct GSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    uint face : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void GSMain(triangle PSInput input[3], inout TriangleStream<GSOutput> stream)
{
    GSOutput output;
    output.face = face;
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        output.position = input[i].position;
        output.normal = input[i].normal;
        stream.Append(output);
    }

}


float4 PSMain(GSOutput input) : SV_TARGET
{

    float3 irradiance = float3(0.0,0.0,0.0);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, input.normal));
    up = normalize(cross(input.normal, right));
    float3 normal = normalize(input.normal);
    //return float4(1.0, 1.0, 0.0, 1.0);
    int raycount = 0;

    float delta = 0.039f;
    for (float phi = 0.0; phi < (2.0 * PI); phi = phi + delta)
    {
    
        for (float theta = 0.0; theta < (0.5 * PI); theta = theta+ delta)
        {
            float3 tangentvector = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

            float3 dir = -1*(tangentvector.x * right + tangentvector.y * up + tangentvector.z * normal);
            irradiance += cube.Sample(g_sampler, dir).rgb * cos(theta) * sin(theta);
            raycount = raycount +1;

        }
    }

    float3 final = PI*irradiance / ((float) raycount );
  //  final = final / (1 + final);
    return float4(final, 1.0);

 //   return float4(raycount, 1.0, 1.0, 1.0);
//     return cube.Sample(g_sampler, input.normal);


	//return float4(input.normal,1.0f);
}
