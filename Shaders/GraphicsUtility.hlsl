struct Material
{
    float roughness;
    float metallic;
    float3 albedo;
    uint4 texutres;
    float4 chooses;
};
struct Matrices
{
    float4x4 model;
    float4x4 normal;
};
struct VertexBufferView
{
    uint2 gpuAddress;
    uint size;
    uint stride;
};
struct IndexBufferView
{
    uint2 gpuAddress;
    uint size;
    uint format;
};