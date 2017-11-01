cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
};
cbuffer InstanceData : register(b1)
{
    float4x4 transform; // 128 byte
    float4x4 normal;


};
struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};
PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL)
{
	PSInput result;
    result.position = mul(transform, float4(position, 1.0f));
    result.position = mul(view, result.position);
	result.position = mul(proj,result.position);
	result.normal = normal;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{

	return float4(input.normal,1.0f);
}
