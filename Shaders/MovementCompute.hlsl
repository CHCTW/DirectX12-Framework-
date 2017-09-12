cbuffer SceneConstantBuffer : register(b0)
{
	float off;
};

struct PosOffset
{
	float2 offset;
	float3 color;
};
RWStructuredBuffer<PosOffset> BufferOut : register(u0);
[numthreads(4, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float x = BufferOut[DTid.x].offset.x;
	float y = BufferOut[DTid.x].offset.y;
	BufferOut[DTid.x].offset.x = x*cos(off)-y*sin(off);
	BufferOut[DTid.x].offset.y = y*cos(off) + x*sin(off);
	BufferOut[DTid.x].offset = normalize(BufferOut[DTid.x].offset)/3;
}