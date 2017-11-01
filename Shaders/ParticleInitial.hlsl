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
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}


RWStructuredBuffer<Particle> ParticleBuffer : register(u0);
[numthreads(512, 1, 1)]

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

        float2 xz = Hammersley(id.x, totalNum);


        ParticleBuffer[id.x].Pos = float3(0, 0, 0);
        ParticleBuffer[id.x].Vel = float3(0, 0,0);
        
        ParticleBuffer[id.x].life = ((float) id / (float)totalNum) * 10;
        ParticleBuffer[id.x].Color = float3(0.0, 0, 0);
    }

}