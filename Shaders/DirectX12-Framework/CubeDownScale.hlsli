//A head file for mipmap chain code
// approach do gaussian filter first, then do subsample

#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define PI 3.14159

//#define SRGB_A
//#define GAUSSIAN_FILTER
//#define SRGB_A
/**************************

combiniation : data type + filter type + address mode

data type:

#define SRGB_A  // 4 channel
#define RGBA_LINEAR // can use in hdr mode or other....

filter type:
#define DISC_FILTER
#define GAUSSIAN_FILTER



example SRGBA+BOX_FILTER

**************************/


#define BlockSize 16
#define gaussinwindow 5
#define FOURCHANNELDATA 


static float3 faceup[6] =
{
    float3(1.0, 0.0f, 0.0f),
    float3(0.0, 1.0f, 0.0f),
    float3(0.0, 0.0f, 1.0f),
    float3(1.0, 0.0f, 1.0f),
    float3(0.0, 1.0f, 1.0f),
    float3(1.0, 1.0f, 0.0f)
};


cbuffer Camera : register(b0)
{
    //uint SrcSliceNum;
    CubeCamera cubeCamera; // Texture level of source mip
}

cbuffer mipdata : register(b1)
{
    //uint SrcSliceNum;
    uint srcMipLevel; // Texture level of source mip
}

//#define RGBA_NORMAL     // temporary for test
//#define GAUSSIAN_FILTER  // temporary for test, the very basic way to down scale


//defined(SRGB_ALPHA_MASK)||defined(SRGB_A)||defined(RGBA_LINEAR)||defined(SRGB_ALPHA_TRANSPARENCY)||defined(RGBA_NORMAL_VECTOR) // all 4 channel data




#if defined(SRGB_A)||defined(RGBA_LINEAR)
TextureCube<float4> SrcTexture : register(t0);
RWTexture2DArray<float4> DesTexture : register(u0);


#endif

SamplerState BiSampler : register(s0);




#if defined(DISC_FILTER)
void discfilterandwrite(float4 data[17],float cospow, uint2 pos, uint face)
{
    float4 res = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float total = 0.0f;
#if defined(SRGB_A)
    [unroll]
    for (uint i = 0; i < 17; ++i)
    {
        if(i==0)
            res += pow(data[i], 2.2f);
        else
            res += pow(data[i], 2.2f)*cospow;
    }
    res /= (cospow*16+1.0f);
    DesTexture[uint3(pos, face)] = pow(res, 1.0f / 2.2f);
#endif
#if defined(RGBA_LINEAR)
    [unroll]
    for (uint i = 0; i < 17; ++i)
    {
        if(i==0)
            res += data[i];
        else
            res += data[i]*cospow;
    }
    res /= (cospow*16+1.0f);
    DesTexture[uint3(pos, face)] = res;
#endif
}

#endif



#if defined(GAUSSIAN_FILTER)
static float gaussweight[3] =
{ 0.38774, 0.24477, 0.06136 };
//{ 0.2, 0.45, 0.05 };




static float totalinv = 1.0f;
static const int gausswidth = 3;
void gaussianfilterandwrite(float4 data[3], uint2 pos, uint face)
{
	
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float totalweight = 0.0f;
    for (int i = 0; i < gausswidth; ++i)
    {
        totalweight += gaussweight[i];
        result += gaussweight[i] * data[i];
    }
    result /= totalweight;
#if defined(SRGB_A)
	 DesTexture[uint3(pos, face)] = pow(result, 1.0f / 2.2f);
#endif
#if defined(RGBA_LINEAR)
    DesTexture[uint3(pos, face)] = result;
#endif
}


#endif




float3 getViewPos(float2 uv, uint face)
{
    float4 projcoord;
    projcoord.xy = uv;
    projcoord.y = 1.0 - projcoord.y;
    projcoord.xy = projcoord.xy * 2.0f - 1.0f;
    projcoord.z = 1.0f;
    projcoord.w = 1.0f;

    float4 vpos = mul(cubeCamera.projectionviewinverse[face], projcoord);
    vpos.xyzw /= vpos.w;
    return vpos.xyz;
}


[numthreads(BlockSize, BlockSize, 1)]
void CSMain(uint3 gid : SV_GroupID, uint3 tid : SV_DispatchThreadID) // tid is the position of out put texture
{
    int3 pos = tid;
    pos.z = 0;
    uint3 dim;
    uint totalmip;
    uint face = gid.z;


    SrcTexture.GetDimensions(srcMipLevel + 1, dim.x, dim.y, totalmip);
        
    if (all(pos.xy < dim.xy))
    {
    
    

        float2 fdim = dim.xy;
  //      float2(1.0f,1.0f);
   // float scale = (fdim.x - 1) / fdim.x;

        float2 uv = float2(tid.xy);
        
    //uv += uv / (fdim - float2(1.0f,1.0f));
        float2 uvnear = float2(tid.xy) + float2(1.0f,0.0f);
        uv += float2(0.5f, 0.5f);
        uv /= (fdim);
        float para = (abs(uv.x - 0.5f) + abs(uv.y - 0.5f))*0.5f
        +1.0f;

   // uv.x = clamp(uv.x, 0.0f, 1.0f);
  //  uv.y = clamp(uv.y, 0.0f, 1.0f);

        //uvnear += float2(0.5f, 0.5f);
        uvnear /= (fdim);

        float3 vpos = getViewPos(uv, face);



    //float M = max(max(abs(vpos.x), abs(vpos.y)), abs(vpos.z));
    //float scale = (fdim.x - 1) / fdim.x;
    //if (abs(vpos.x) != M)
    //    vpos.x *= scale;
    //if (abs(vpos.y) != M)
    //    vpos.y *= scale;
    //if (abs(vpos.z) != M)
    //    vpos.z *= scale;


        float3 vposnear = getViewPos(uvnear, face);

        float len = length(vpos);
    //len = 10.0f;
        float3 vdir = normalize(vpos);
        float3 vdirnear = normalize(vposnear);
  
        float angleoffset = abs(acos(dot(vdir, vdirnear))) * para;

     //   angleoffset = (PI * 0.5f) / (fdim.x) * para;
     //   len = 1.0f;
	// get radius
#ifdef DISC_FILTER
        float radius = tan(angleoffset) * len;
        float cospow = pow(dot(vdir, vdirnear),32);

#endif

#ifdef GAUSSIAN_FILTER
        float radius = tan(angleoffset) * len * para * clamp((srcMipLevel - 5.0f) / 3.0f,
        1.0f, 5.0f);

#endif

        float3 tangent = normalize(cross(faceup[face], vdir));
        float3 bitangent = normalize(cross(vdir, tangent));

#ifdef DISC_FILTER
    float discangleoffset = (2 * PI) / 16.0f;

#if defined(SRGB_A)||defined(RGBA_LINEAR)// all 4 channel data
    float4 data[17];
#endif


    // no mether what type of data in source input, still going to read them all
    float phi = 0.0f;
    [unroll]
    for (uint i = 0; i < 16; ++i)
    {

        float3 view = vpos + cos(phi) * tangent * radius + sin(phi) * bitangent * radius;
        data[i + 1] = SrcTexture.SampleLevel(BiSampler, view, srcMipLevel);
        phi += discangleoffset;
    }
    data[0] = SrcTexture.SampleLevel(BiSampler, vpos, srcMipLevel);
    discfilterandwrite(data, cospow,pos.xy, face);
#endif

#ifdef GAUSSIAN_FILTER
    float4 data[3];
    float phi = 0.0f;
    data[0] = float4(0.0, 0.0, 0.0, 0.0);
    data[1] = float4(0.0, 0.0, 0.0, 0.0);
    data[2] = float4(0.0, 0.0, 0.0, 0.0);
    float discangleoffset = (2 * PI) / 8.0f;
	[unroll]
    for (uint i = 0; i < 8; ++i)
    {
        float3 view = vpos + cos(phi) * tangent * radius + sin(phi) * bitangent * radius;
#if defined(SRGB_A)
		 data[1] += pow(SrcTexture.SampleLevel(BiSampler, view, srcMipLevel),2.2f);
#endif
#if defined(RGBA_LINEAR)
        data[1] += SrcTexture.SampleLevel(BiSampler, view, srcMipLevel);
#endif
        phi += discangleoffset;
    }
    data[1] /= 8.0f;


    discangleoffset = (2 * PI) / 16.0f;
    radius *= 2;
    phi = 0.0f;
	[unroll]
    for (uint i = 0; i < 16; ++i)
    {
        float3 view = vpos + cos(phi) * tangent * radius + sin(phi) * bitangent * radius;
#if defined(SRGB_A)
		 data[2] += pow(SrcTexture.SampleLevel(BiSampler, view, srcMipLevel),2.2f);
#endif
#if defined(RGBA_LINEAR)
        data[2] += SrcTexture.SampleLevel(BiSampler, view, srcMipLevel);
#endif
        phi += discangleoffset;
    }
    data[2] /= 16.0f;
#if defined(SRGB_A)
		 data[0] += pow(SrcTexture.SampleLevel(BiSampler, vpos, srcMipLevel),2.2f);
#endif
#if defined(RGBA_LINEAR)
    data[0] += SrcTexture.SampleLevel(BiSampler, vpos, srcMipLevel);
#endif    
    gaussianfilterandwrite(data, pos.xy, face);

#endif
    //  DesTexture[uint3(pos.xy, face)] = float4(vdir, 0.0f);
    }
}