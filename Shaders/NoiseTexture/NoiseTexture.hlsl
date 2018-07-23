struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);
PSInput VSMain(float2 position : POSITION, float2 uv : TEXCOORD, uint id : SV_VertexID)
{
	PSInput result;
	result.uv = float2((id << 1) & 2, id & 2);
	result.position = float4(result.uv  * float2(2, -2) + float2(-1, 1), 0, 1);


	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{

    return pow(g_texture.Sample(g_sampler, input.uv*2.0f).xxxx, 1.0 / 2.2f);
}
