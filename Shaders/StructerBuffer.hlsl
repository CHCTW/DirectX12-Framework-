struct PosOffset
{
	float2 offset;
	float3 color;
};
struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	uint instanceid :ID;
};
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);
StructuredBuffer<PosOffset> Offsets: register(t1);

PSInput VSMain(float2 position : POSITION, float2 uv : TEXCOORD,uint instanceid : SV_InstanceID)
{
	PSInput result;
	position = position+Offsets[instanceid].offset;
	result.position = float4(position, 1.0, 1.0);
	result.uv = uv;
	result.instanceid = instanceid;
	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	
	return float4(Offsets[input.instanceid].color,1.0)*g_texture.Sample(g_sampler, input.uv);
}
