void generateNomralizePlane(in float3 points[3],out float4 plane) // winding is important here, please consider the noraml vector for the plane
{
    float3 vec1 = (points[1] - points[0]);
    float3 vec2 = (points[2] - points[0]);
    float3 normal = cross(vec1, vec2);
    normal = normalize(normal);
    float d = -dot(normal, points[0]);
    plane.xyz = normal;
    plane.w = d;
}
void generateUnomralizePlane(in float3 A, in float3 B, in float3 C, out float4 plane) // winding is important here, please consider the noraml vector for the plane
{
    float3 vec1 = (B - A);
    float3 vec2 = (C - A);
    float3 normal = cross(vec1, vec2);
  //  normal = normalize(normal);
    float d = -dot(normal, A);
    plane.xyz = normal;
    plane.w = d;
}


bool InsidePlane(in float3 testpoint,in float4 plane)
{
    return (dot(float4(testpoint, 1.0f), plane) >= 0);
}
void generateCorner(float4 min, float4 max, out float4 cornor[8], in float4x4 Transform) 
{
  //  float4 c[8];
    cornor[0] = mul(Transform, float4(max.x, max.y, max.z, 1)); // top-right-back
    cornor[1] = mul(Transform, float4(max.x, max.y, min.z, 1));
    cornor[2] = mul(Transform, float4(max.x, min.y, max.z, 1));
    cornor[3] = mul(Transform, float4(min.x, max.y, max.z, 1));
    cornor[4] = mul(Transform, float4(max.x, min.y, min.z, 1));
    cornor[5] = mul(Transform, float4(min.x, max.y, min.z, 1));
    cornor[6] = mul(Transform, float4(min.x, min.y, max.z, 1));
    cornor[7] = mul(Transform, float4(min.x, min.y, min.z, 1));
    //corner = c;
}
void generateAlignBox(in float4 cornor[8],out float3 minedge,out float3 maxedge)
{
    minedge = cornor[0].xyz;
    maxedge= cornor[0].xyz;
    [unroll]
    for (int i = 1; i < 8; ++i)
    {
        maxedge = max(cornor[i].xyz, maxedge);
        minedge = min(cornor[i].xyz, minedge);
    }

}
bool AlignBoxisSphereIntsec(in float3 mines,in float3 maxes,in float3 center,in float radius)
{
    float3 closest = max(mines - center, 0.0) + max(center - maxes, 0.0);
    return (dot(closest, closest) <= radius * radius);
}
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
static uint indexlist[36] = // this is the content of index buffer for the unit cube
{
    0, 2, 1, 0, 3, 2, // back
    4, 5, 6, 4, 6, 7, // front
    0, 4, 7, 0, 7, 3, // right
    1, 6, 5, 1, 2, 6, // left
    0, 1, 5, 0, 5, 4, // top
    2, 3, 6, 3, 7, 6 // bot
};
static float2 quadpoints[4] = // 
{
    float2(-1.0, 1.0),
    float2(-1.0, -1.0),
    float2(1.0, 1.0),
    float2(1.0, -1.0),
};
static uint quadindexlist[6] =
{
    0, 1, 2,
    2, 1, 3
};

float3 getUnitCubePoint(in uint index)
{
    return cubepoints[indexlist[index]];
}
float2 getUnitQuadPoint(in uint index)
{
    return quadpoints[quadindexlist[index]];
}


bool SphereInsideNormalizePlane(in float4 plane, in float3 center, in float radius)
{
    float3 furthest = center + plane.xyz * radius;
    float side = dot(plane.xyz, furthest) + plane.w;
    if (side >= 0)
        return true;
    else
        return false;
}
bool FlatPointInsideTriangle(in float3 A, in float3 B, in float3 C, float3 p)
{
    float3 vec1 = A - B;
    float3 vec2 = A - C;
    float3 vec3 = A - p;
    float d11 = dot(vec1, vec1);
    float d12 = dot(vec1, vec2);
    float d22 = dot(vec2, vec2);
    float d31 = dot(vec3, vec1);
    float d32 = dot(vec3, vec2);
    float denom = 1.0f / (d11 * d22 - d12 * d12);
    float v = (d22 * d31 - d12 * d32) * denom;
    float w = (d11 * d32 - d12 * d31) * denom;
    float u = 1.0f - v - w;
    return (v >= -1.5f);

}

float3 findPojectionPoint(in float4 plane,in float3 p)
{
    float t = dot(plane, float4(p, 1.0)) / dot(plane.xyz, plane.xyz);
    return p - t * plane.xyz;

}
inline bool IsSphereTriangleIntersectBackCull(in float3 A, in float3 B, in float3 C,in float3 center,in float radius)
{
    float4 plane = float4(1.0,1.0,1.0,1.0);
    generateUnomralizePlane(A, B, C, plane);
    float3 viewdir = center - A;
    bool face = (dot(plane.xyz, viewdir) >= 0.0f);
    float3 projectionpoint = findPojectionPoint(plane, center);
    float3 vec = projectionpoint - center;
    float rsquare = radius * radius;
    bool intersect =  dot(vec, vec) <= rsquare;
    vec = A - center; // check all three point, whehter it is inside sphere
    bool inside = dot(vec, vec) <= rsquare;
    vec = B - center;
    inside = inside || dot(vec, vec) <= rsquare;
    vec = C - center;
    inside = inside || dot(vec, vec) <= rsquare;
    bool intri = FlatPointInsideTriangle(A, B, C, projectionpoint); // check whetbher porjection point is in triangle
    return face && (inside || (intri && intersect)); 

}


