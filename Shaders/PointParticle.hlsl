cbuffer SceneConstantBuffer : register(b0)
{
	float x;
	float y;
	float size;
	float r;
	float g;
	float b;
	float a;
};
struct PSInput
{
	float4 position : SV_POSITION;
    float ndcpos : NDC;
	float2 uv : TEXTCOORD;
};
static float2 offset[4] =
{
	float2(-0.25f,-0.25f),
	float2(-0.25f,0.25f),
	float2(0.25f,-0.25f),
	float2(0.25f,0.25f)
};
static float2 uv[4] =
{
    float2(0.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};
PSInput VSMain(uint id : SV_VertexID)
{
	PSInput result;
    result.position = float4(x + offset[id].x * size, y + offset[id].y * size, 1.0f, 1.0f);
    result.ndcpos = result.position.xy;
    result.uv = uv[id];


	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float falloff = 1.0f-length(input.uv- float2(0.5f, 0.5f)) / 0.5f;
    return float4(r * a * falloff, g * a * falloff, b * a * falloff, 1.0f);
    //return float4(input.uv, 0.0, 1.0);
}
