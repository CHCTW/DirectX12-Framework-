struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);
PSInput VSMain(float2 position : POSITION, float2 uv : TEXCOORD)
{
	PSInput result;
	result.position = float4(position,1.0,1.0);
	result.uv = uv;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 col = g_texture.Sample(g_sampler, input.uv);
    if(col.a==0)
        discard;
	return g_texture.Sample(g_sampler, input.uv);
}
