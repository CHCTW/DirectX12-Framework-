struct Particle
{
    float3 Pos;
    float3 Vel;
    float3 Color;
    float Life;
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float3 color : COLOR;
   
};
cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 proj;
};
StructuredBuffer<Particle> Particles : register(t0);
VSOut VSMain(uint id : SV_VertexID)
{
    VSOut output;
    float4 p = float4(Particles[id].Pos, 1.0);
    output.pos = mul(view, p);
    output.pos = mul(proj, output.pos);
    output.color = Particles[id].Color;

    return output;
}
float4 PSMain(VSOut vsout) : SV_TARGET
{
    return float4(vsout.color, 1.0);
}