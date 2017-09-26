cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
};
cbuffer WindowSize : register(b1)
{
    float width;
    float height;
};


TextureCube cube : register(t0);
Texture2D depth : register(t1);
SamplerState g_sampler : register(s0);
struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL)
{
	PSInput result;
	result.normal = position.xyz;
	result.position = mul(view,float4(position*18000,1.0f)); 
	result.position = mul(proj,result.position);
    result.position.z = result.position.w;
//	result.normal = normal;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{

    
//    return cube.Sample(g_sampler, input.normal);
 //   if(input.position.z>=0.99f)


    float2 coord = float2(input.position.x /width, (input.position.y /height));

  //  return float4(coord, 0.0, 1.0);

    float d = depth.Sample(g_sampler, coord).r * 5;
    d = d / 1 + d;
//    return float4(d/50, d/50, d/50, 1.0);
    if (depth.Sample(g_sampler,coord).r<1.0)
        discard;
        return pow(cube.SampleLevel(g_sampler, input.normal,0.0) / (1 + cube.SampleLevel(g_sampler, input.normal,0.0)), (1 / 2.2f));

	//return float4(input.normal,1.0f);
}
