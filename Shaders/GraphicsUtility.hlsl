// This struture should be the same Materil.h
struct Material
{
    float roughness;
    float metallic;
    float3 albedo;
    uint4 texutres;
    float4 chooses;
};
// This structure should be the same as Matrices in Transfrom.h
struct Matrices
{
    float4x4 model;
    float4x4 normal;
};
// This structure should be the same as Object.h
struct Object
{
    uint materialid;
    uint meshid;
    uint matricesid;
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
// This structer should match with Mesh.h
struct Mesh
{
    VertexBufferView position;
    VertexBufferView normal;
    VertexBufferView uv;
    VertexBufferView tangent;
    IndexBufferView index;
    uint indexCount;
    uint startIndex;
    float3 min; // bounding box
    float3 max;
};
struct Camera
{
    float4x4 view;
    float4x4 proj;
    float3 eye;
    float padding;
    float4x4 viewinverse;
    float4x4 projinverse;
    float4 mFrustumPlane[6];
    float front;
    float back;
};
float3 normalMapCal(in float rate,in float3 objnormal,in float3 tangent,in float3 textnormal)
{
    float3 normalobject = normalize(objnormal);
    float3 tan = normalize(tangent);
    float3 bitan = normalize(cross(tan, normalobject));
    return  textnormal.x * tan + textnormal.y * bitan + textnormal.z * normalobject;
    
}
struct GBufferFormat
{
    half2 normal : COLOR0;
    float4 albedorough : COLOR1;
    float4 emmermetal : COLOR2;
};


//Enconde and decode a two channel coordinate,the method is used in cryengine 3
// I found in this website https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// a very nice website that have various method to reduce the stroge space of gbuffer
half2 encode(float3 n)
{
    half2 enc = normalize(n.xy) * (sqrt(-n.z * 0.5 + 0.5));
    enc = enc * 0.5 + 0.5;
    return enc;
}
float3 decode(half2 enc)
{
    half4 nn = float4(enc, 0.0, 0.0) * float4(2, 2, 0, 0) + float4(-1, -1, 1, -1);
    half l = dot(nn.xyz, -nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(l);
    return nn.xyz * 2 + half3(0, 0, -1);
}

//use method http://research.nvidia.com/publication/2d-polyhedral-bounds-clipped-perspective-projected-3d-sphere
// to generate bounding 2d box for sphere

void BoundsforAxis(in float3 axis, in float3 center, float radius, float near, out float3 L, out float3 U)
{
    float2 c = float2(dot(axis, center), center.z);

    float tsquare = dot(c, c) - radius * radius;
    bool cameraindside = (tsquare <= 0);

    float2 bounds[2];
    float2 v;
    if (tsquare <= 0)
        v = float2(0.0, 0.0);
    else
        v = float2(sqrt(tsquare), radius) / length(c);
    bool clipsphere = (center.y + radius >= near);

    float k = sqrt(radius * radius - (near - c.y) * (near - c.y));
    [unroll]
    for (int i = 0; i < 2; ++i)
    {
    
        if (!cameraindside)
            bounds[i] = mul(float2x2(v.x, -v.y, v.y, v.x), c.xy) * v.x;
        bool clipbound = (cameraindside || (bounds[i].y > near));
        if (clipsphere && clipbound)
            bounds[i] = float2(c.x + k, near);
        v.y = -v.y;
        k = -k;
    }
    L = bounds[1].x * axis;
    L.z = bounds[1].y;

    U = bounds[0].x * axis;
    U.z = bounds[0].y;
}