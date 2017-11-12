static float3 cubepoints[8] =
{
    float3(1.0f, 1.0f, 1.0f),
    float3(-1.0f, 1.0f, 1.0f),
    float3(-1.0f, -1.0f, 1.0f),
    float3(1.0f, -1.0f, 1.0f),

    float3(1.0f, 1.0f, -1.0f),
    float3(-1.0f, 1.0f, -1.0f),
    float3(-1.0f, -1.0f, -1.0f),
    float3(1.0f, -1.0f, -1.0f),


};
static uint indexlist[36] =
{
    0, 2, 1, 0, 3, 2, // back
    4, 5, 6, 4, 6, 7, // front
    0, 4, 7, 0, 7, 3, // right
    1, 6, 5, 1, 2, 6, // left
    0, 1, 5, 0, 5, 4, // top
    2, 3, 6, 3, 7, 6 // bot
};
float3 getUnitCubePoint(in uint index)
{
    return cubepoints[indexlist[index]];
}