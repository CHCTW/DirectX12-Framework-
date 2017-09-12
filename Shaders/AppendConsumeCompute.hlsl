cbuffer SceneConstantBuffer : register(b0)
{
	float off;
};

struct PosOffset
{
	float2 offset;
	float3 color;
};
ConsumeStructuredBuffer<PosOffset> BufferIn : register(u0);
AppendStructuredBuffer<PosOffset> BufferOut : register(u1);
[numthreads(4, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

	PosOffset data = BufferIn.Consume();
	float x = data.offset.x;
	float y = data.offset.y;



	data.offset.x = x*cos(off)-y*sin(off);
	data.offset.y = y*cos(off) + x*sin(off);
	data.offset = normalize(data.offset)/5;
	BufferOut.Append(data);
}