struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
Texture2D littexture : register(t0);
Texture2D bloomtexure : register(t1);
SamplerState g_sampler : register(s0);
PSInput VSMain(uint id : SV_VertexID)
{
	PSInput result;
	result.uv = float2((id << 1) & 2, id & 2);
	result.position = float4(result.uv  * float2(2, -2) + float2(-1, 1), 0, 1);


	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	//return float4(input.uv,0.0f,0.0f);
    float4 final = littexture.Sample(g_sampler, input.uv) + bloomtexure.Sample(g_sampler, input.uv);
    final = final / (1.0f + final);
    pow(final, 1.0f / 2.2f);
  //  final = pow(final, 2.2f);
    return final;
}
