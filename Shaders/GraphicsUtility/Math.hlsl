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
#define MAXRADIUS 50
float getGaussionWeight(in float4 array[(MAXRADIUS * 2 + 1 + 3) / 4], uint index)
{
    uint entry = index / 4;
    uint channel = index % 4;
    return array[entry][channel];
}
struct GaussionData
{

    uint radius;
    float3 padding;
    float4 weight[(MAXRADIUS * 2 + 1+3)/4];
};

// a random 2d function from https://thebookofshaders.com/12/ 
float2 random2(float2 p)
{
    return frac(sin(float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)))) * 43758.5453);
}

// functions for perlin noise from http://flafla2.github.io/2014/08/09/perlinnoise.html
static int permutation[512] = { 151,160,137,91,90,15,					// Hash lookup table as defined by Ken Perlin.  This is a randomly
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,	// arranged array of all numbers from 0-255 inclusive.
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91, 90, 15, // Hash lookup table as defined by Ken Perlin.  This is a randomly
		131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, // arranged array of all numbers from 0-255 inclusive.
		190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
		88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
		77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
		102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
		135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
		5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
		223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
		129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
		251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
		49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
		138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};
float gradient(int hashnumber,float x, float y,float z)
{
    int hash = hashnumber & 15;
    float u;
    if (hash<8)
        u = x;
    else
        u = y;
    float v;
    if (hash<4)
        v = y;
    else if (hash==12||hash==14)
        v = x;
    else
        v = z;
    if(hash & 1!=0)
        u = -u;
    if (hash & 2 != 0)
        v = -v;
    return u + v;

}
float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}
float lerpforfrac(float a, float b,float x)
{
    return a + x * (b - a);
}
float repeatinc(float x,int repeat)
{
    float inc = x+1;
    if (repeat > 0)
        inc = fmod(inc, (float)repeat);
    return inc;
}
float perlinnoise(float x, float y, float z, int repeat)
{
    float rx = x;
    float ry = y;
    float rz = z;
    if (repeat>0)
    {
        rx = fmod(rx, (float)repeat);
        ry = fmod(ry, (float)repeat);
        rz = fmod(rz, (float)repeat);
    }
    int xi = int(floor(rx)) & 255;
    int yi = int(floor(ry)) & 255;
    int zi = int(floor(rz)) & 255;
    float xf = frac(rx);
    float yf = frac(ry);
    float zf = frac(rz);
    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);
    int aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = permutation[permutation[permutation[xi] + yi] + zi];
    aba = permutation[permutation[permutation[xi] + repeatinc(yi,repeat)] + zi];
    aab = permutation[permutation[permutation[xi] + yi] + repeatinc(zi,repeat)];
    abb = permutation[permutation[permutation[xi] + repeatinc(yi, repeat)] + repeatinc(zi, repeat)];
    baa = permutation[permutation[permutation[repeatinc(xi, repeat)] + yi] + zi];
    bba = permutation[permutation[permutation[repeatinc(xi, repeat)] + repeatinc(yi, repeat)] + zi];
    bab = permutation[permutation[permutation[repeatinc(xi, repeat)] + yi] + repeatinc(zi, repeat)];
    bbb = permutation[permutation[permutation[repeatinc(xi, repeat)] + repeatinc(yi, repeat)] + repeatinc(zi, repeat)];
    float x1, x2, y1, y2;
    x1 = lerpforfrac(gradient(aaa, xf, yf, zf), // The gradient function calculates the dot product between a pseudorandom
					gradient(baa, xf - 1, yf, zf), // gradient vector and the vector from the input coordinate to the 8
					u);
    x2 = lerpforfrac(gradient(aba, xf, yf - 1, zf), // This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
					gradient(bba, xf - 1, yf - 1, zf), // values we made earlier.
			          u);
    y1 = lerpforfrac(x1, x2, v);
    x1 = lerpforfrac(gradient(aab, xf, yf, zf - 1),
					gradient(bab, xf - 1, yf, zf - 1),
					u);
    x2 = lerpforfrac(gradient(abb, xf, yf - 1, zf - 1),
		          	gradient(bbb, xf - 1, yf - 1, zf - 1),
		          	u);
    y2 = lerpforfrac(x1, x2, v);
    return (lerpforfrac(y1, y2, w) + 1.0f) / 2.0f;

}
float perlinnoise2d(float x, float y, float z, int repeat)
{
    float rx = x;
    float ry = y;
    float rz = z;
    if (repeat > 0)
    {
        rx = fmod(rx, (float) repeat);
        ry = fmod(ry, (float) repeat);
    }
    int xi = int(floor(rx)) & 255;
    int yi = int(floor(ry)) & 255;
    int zi = int(floor(rz)) & 255;
    float xf = frac(rx);
    float yf = frac(ry);
    float zf = frac(rz);
    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);
    int aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = permutation[permutation[permutation[xi] + yi] + zi];
    aba = permutation[permutation[permutation[xi] + repeatinc(yi, repeat)] + zi];
    aab = permutation[permutation[permutation[xi] + yi] + repeatinc(zi, 255)];
    abb = permutation[permutation[permutation[xi] + repeatinc(yi, repeat)] + repeatinc(zi, 255)];
    baa = permutation[permutation[permutation[repeatinc(xi, repeat)] + yi] + zi];
    bba = permutation[permutation[permutation[repeatinc(xi, repeat)] + repeatinc(yi, repeat)] + zi];
    bab = permutation[permutation[permutation[repeatinc(xi, repeat)] + yi] + repeatinc(zi, 255)];
    bbb = permutation[permutation[permutation[repeatinc(xi, repeat)] + repeatinc(yi, repeat)] + repeatinc(zi, 255)];
    float x1, x2, y1, y2;
    x1 = lerpforfrac(gradient(aaa, xf, yf, zf), // The gradient function calculates the dot product between a pseudorandom
					gradient(baa, xf - 1, yf, zf), // gradient vector and the vector from the input coordinate to the 8
					u);
    x2 = lerpforfrac(gradient(aba, xf, yf - 1, zf), // This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
					gradient(bba, xf - 1, yf - 1, zf), // values we made earlier.
			          u);
    y1 = lerpforfrac(x1, x2, v);
    x1 = lerpforfrac(gradient(aab, xf, yf, zf - 1),
					gradient(bab, xf - 1, yf, zf - 1),
					u);
    x2 = lerpforfrac(gradient(abb, xf, yf - 1, zf - 1),
		          	gradient(bbb, xf - 1, yf - 1, zf - 1),
		          	u);
    y2 = lerpforfrac(x1, x2, v);
    return (lerpforfrac(y1, y2, w) + 1.0f) / 2.0f;
}
