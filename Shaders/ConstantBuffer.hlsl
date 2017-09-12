
cbuffer SceneConstantBuffer : register(b0)
{
	float offset;
};

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

	result.position = float4(position.x+offset, position.y,1.0,1.0);
	result.uv = uv;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	//return float4(input.uv,0.0f,0.0f);
	float inten = offset / 2 + 0.5;
	float redinten = inten;
	float greeninten = inten * 2;
	return float4(redinten,greeninten,0.0f,0.0f)*g_texture.Sample(g_sampler, input.uv);
}
