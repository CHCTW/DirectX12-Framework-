cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
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
	result.normal = position.xyz;
	result.position = mul(view,float4(position*18000,1.0f)); 
	result.position = mul(proj,result.position);
    result.position.z = result.position.w;
//	result.normal = normal;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return cube.Sample(g_sampler,input.normal);
	//return float4(input.normal,1.0f);
}
