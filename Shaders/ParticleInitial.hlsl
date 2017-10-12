struct Particle
{
    float3 Pos;
    float3 Vel;
    float3 Color;
    float life;
};
cbuffer SceneConstBuffer : register(b0)
{
    float delta;
    uint totalNum;
};
float rand_1_05(float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}



RWStructuredBuffer<Particle> ParticleBuffer : register(u0);
[numthreads(128, 1, 1)]

void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID)
{
    if(id.x < totalNum) //
    {
        Particle data;
        float ran = rand_1_05(float2(id.x,tid.x));
        float ran2 = rand_1_05(float2(id.x, tid.x+1));
       float ran3 = rand_1_05(float2(id.x, tid.x-1));
  //    data.Pos = float3(0, 0, 0);
   //     data.Color = float3(1, 0, 1);
        ParticleBuffer[id.x].Pos = float3(0, 0, 0);
        ParticleBuffer[id.x].Vel = float3(ran - 0.5, (ran2 + 0.3) * 3, ran3 - 0.5);
        
        ParticleBuffer[id.x].life = ((float) id / totalNum)*15;
        ParticleBuffer[id.x].Color = float3(0.1, ParticleBuffer[id.x].life / 15, 1-ParticleBuffer[id.x].life / 15);
    }

}