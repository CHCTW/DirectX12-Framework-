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
    uint ParNum;
};
RWStructuredBuffer<Particle> NextState : register(u0);
StructuredBuffer<Particle> PrevState : register(t0);

float rand_1_05(float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}


[numthreads(1024, 1, 1)]
void CSMain(uint id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID)
{
    if (id.x < ParNum)
    {
        if (PrevState[id.x].life > 10)
        {

            float ran = rand_1_05(float2(id.x, tid.x));
            float ran2 = rand_1_05(float2(id.x, tid.x + 1));
            float ran3 = rand_1_05(float2(id.x, tid.x - 1));
  

            NextState[id.x].Pos = float3(0, 0, 0);
            NextState[id.x].Vel = float3(ran - 0.5, (ran2 + 0.3) * 3, ran3 - 0.5);
           // NextState[id.x].Color = PrevState[id.x].Color;
            NextState[id.x].life = 0.0;
            NextState[id.x].Color = float3(0.1, NextState[id.x].life / 15, 1 - NextState[id.x].life / 15);

        }
        else
        {
            NextState[id.x].Pos = PrevState[id.x].Pos + PrevState[id.x].Vel * 0.01;
            NextState[id.x].Vel = PrevState[id.x].Vel - float3(0, 0.01, 0);
     //       NextState[id.x].Color = PrevState[id.x].Color;
            NextState[id.x].life = PrevState[id.x].life + 0.01;
            NextState[id.x].Color = float3(0.1, sqrt(PrevState[id.x].life / 15), 1 - NextState[id.x].life / 15);


        }
        

    }

}