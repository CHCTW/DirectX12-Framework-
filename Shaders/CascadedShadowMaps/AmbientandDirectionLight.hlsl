#include "../GraphicsUtility/Lightfunctions.hlsl"
#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define GROUPSIZE 16
#define MAXSEARCH 8
static float4 debugcolor[8] =
{
    float4(1.0, 0.0, 0.0, 0.0),
    float4(0.0, 1.0, 0.0, 0.0),
    float4(0.0, 0.0, 1.0, 0.0),
    float4(1.0, 1.0, 0.0, 0.0),
    float4(0.0, 1.0, 1.0, 0.0),
    float4(0.0, 1.0, 1.0, 0.0),
    float4(1.0, 1.0, 1.0, 0.0),
    float4(0.0, 0.0, 0.0, 0.0)
};


cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
cbuffer DirectoinLightBuffer : register(b1)
{
    DirectionLightData directionlight;
};
Texture2D GBufferTextures[4] : register(t1);
// decode normal
// albedo + rougbess
//  unused + metlaic
//  depth
Texture2D Silhouette : register(t5);
Texture2DArray ShadowMap : register(t0);
RWTexture2D<float4> HDR : register(u0);
SamplerState pointsampler : register(s0);
float IsEnd(float2 inDisc)
{
    float2 d = (inDisc - 0.75f) * 4.0f;
	
	// x = 0.75 --> 0.0
	// x = 0.5-->-1.0 left or bot side is discont
	// x = 1.0-->1.0  go down or right

    // not discont case
    if (inDisc.x == 0.0f)
        d.x = 0.0f;
    if (inDisc.y == 0.0f)
        d.y = 0.0f;


    return saturate(step(1.0f, dot(abs(d), 1.0f) * 0.5f));
	// d.x+d.y >= 2.0
	// only case that will return 1---> d = (+-1,+-1) --> disk is discountinly in two direction
	//
	//  what is end?
	//     
	//       1
	//    1  1  0   -- > not end  (1.0,0.0)-->not end
	//       1
	//
	//      1
	//   1  1  0   --> end   (1.0,1.0)-->(1.0,1.0)
	//      0
	//
	//   
	//
	//

}
float2 searchend(float slicenum,float4 wpos,float2 shamapdim, float4x4 lightproj,float4x4 lighttoview,float2 ssuv,float2 orgdisc,int2 dir)
{
    float dist = 1.0f;
    float shadow = 0.0f;
    float2 currentuv = ssuv;
    float2 disc = Silhouette.SampleLevel(pointsampler, currentuv, 0);
    float foundEdgeEnd = IsEnd(disc);
    float initialDisc = dot(orgdisc.yx * abs(dir), 1.0f);
    float2 invdim = 1.0f / shamapdim;
    float2 movestep = dir * invdim;


    float4 shadowcoord = mul(lightproj, wpos);
    shadowcoord /= shadowcoord.w;
    shadowcoord.xy = shadowcoord.xy * 0.5 + 0.5;
    shadowcoord.y = 1 - shadowcoord.y;
   // shadowcoord.z = slicenum;
    float2 offset = (dir - frac(shadowcoord.xy * shamapdim) + 0.5f) * invdim;
    float4 currnet = shadowcoord + float4(offset, 0, 0); // point to the center

    currnet.z = slicenum;
    float currentdepth = shadowcoord.z;
    for (int i = 0; i < MAXSEARCH; i++)
    {
      //  float newDepth = tex2D(inShadowMap, currCenterSample.xy).r;
        currnet.z = slicenum;
        float newDepth = ShadowMap.SampleLevel(pointsampler, currnet.xyz, 0);
        float deltaDepth = (currentdepth - newDepth);

        if (abs(deltaDepth) > 0.008)
        {
            if (deltaDepth>=0)
                shadow = 1.0f;
            else
                shadow = 0.0f;
            shadow = 1.0f;
            break;
        }
        currentdepth = newDepth;

        float4 shadowndc = currnet;
        // simple thought what is shadow position on the scrren space coordinate, 
        shadowndc.y = 1.0f - shadowndc.y;
        shadowndc.xy = (shadowndc.xy - 0.5f) * 2.0f;
        shadowndc.z = newDepth;

        float4 ndcview = mul(lighttoview, shadowndc);
        currentuv.xy = ((ndcview.xy / ndcview.w) * 0.5f + 0.5f);
        currentuv.y = 1.0 - currentuv.y;

        disc = Silhouette.SampleLevel(pointsampler, currentuv, 0).rg;
        shadow = Silhouette.SampleLevel(pointsampler, currentuv, 0).b;

        //when searching we probally will face the the siuation that 
        // the last valuabe discon in two axis, but we still want to add this one
        // so, don't care about the axis we are search for
        // since dir is a 1,0 value can use this as a mask
        //  xy->yx means we don't care the dis situation on the axis we are searching

        if (dot(disc.yx * abs(dir), 1.0f) != initialDisc) 
            break;
        if (all(disc.xy == float2(0.0f, 0.0f)))
            break;



        foundEdgeEnd = saturate(foundEdgeEnd + IsEnd(disc));
        dist += 1.0f;
		
		// Go to next shadow map sample
        currnet.xy += movestep;


    }
  //  return disc;
 //   return currentuv;
    return float2(dist, shadow);
//    return lerp(0, dis, foundEdgeEnd);
}
float InterpolationPos(float2 inEdge, float inAxisDisc, float inSubCoord)
{
    float edgeLength = min(dot(abs(inEdge), 1.0f) - 1.0f, MAXSEARCH);
    float2 subCoord = float2(lerp(0, 1 - inSubCoord, saturate(inEdge.x)), lerp(0, inSubCoord, saturate(inEdge.y))) / edgeLength;
    float edgeLengthIsOne = step(0.0f, 1.001f - edgeLength);
	
	// Filter: Sub sample coordinates are only added if the discontinuity ends on the correct side.
    float2 dirFilter = float2(step(inAxisDisc, 0.0f), step(-inAxisDisc, 0.0f));
	
	// Apply filter
    subCoord = subCoord * lerp(float2(1.0f,1.0f), dirFilter, edgeLengthIsOne.xx);
	
	// Create normalized coordinate
    float2 p = (1.0f - (inEdge) / (edgeLength)) * step(0.0f, inEdge) + subCoord;
	
    return max(p.x, p.y);
}
float normalizecoord(float start,float end,float subcoord)
{
    float length = end + start - 1;
    float coord = (start - 1 + subcoord) / length;
    return coord;
}
float refillshadow(float2 subnormcoord, uint4 isshadow)//(left,right,up,down)
{
  //  float leftdown = isshadow
    float xcode = step(0.0, subnormcoord.x - subnormcoord.y);
    float ycode = step(0.0, subnormcoord.x - 1 + subnormcoord.y);

    uint leftdown = isshadow.x | isshadow.w;
    uint leftup = isshadow.x | isshadow.z;
    uint rightdown = isshadow.y | isshadow.w;
    uint rightup = isshadow.y | isshadow.z;

    float res = saturate(lerp(leftdown, rightup, xcode) * lerp(leftup, rightdown,ycode));
    return res;

}
[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, 
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 pos = id.xy;
   


    float3 albedo = pow(GBufferTextures[1][pos].rgb, 2.2);
    float roughness = GBufferTextures[1][pos].a;
    float3 normal = decode(GBufferTextures[0][pos].rg);
    float metallic = GBufferTextures[2][pos].a;
    float depth = GBufferTextures[3][pos].r;
    float2 uv = pos;
    float2 dim;
    HDR.GetDimensions(dim.x, dim.y);
    uv = uv / dim;
    float4 projcoord;
    projcoord.xy = uv;
    projcoord.y = 1.0 - projcoord.y;
    projcoord.xy = projcoord.xy * 2.0f - 1.0f;
    projcoord.z = depth;
    projcoord.w = 1.0f;
    float4 vpos = mul(camera.projinverse, projcoord);
    vpos.xyzw /= vpos.w;
    float4 wpos = mul(camera.viewinverse, vpos);
    wpos /=  wpos.w;



    float4 light = directionlight.lightDirection;
    light.w = 0.0f;
    light = mul(camera.viewinverseyranspose, light); // move light direction to view space

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, float3(metallic, metallic, metallic)); // use metalic value to get F

    float3 N = normal;
    float3 V = normalize(-vpos.xyz);
    float3 L = normalize(-light.xyz);
    float3 H = normalize(L + V);
    float LH = max(dot(L, H), 0.0f);
    float NL = max(dot(L, N), 0.0f);
    float NV = max(dot(N, V), 0.0f);
    float HV = max(dot(H, V), 0.0f);



    float3 F = Fresnel(F0, HV);
    float D = Distribution(N, H, roughness);
    float3 G = GeometrySmith(NV, NL, roughness);

    float3 spec = D * F * G / (4 * NV * NL + 0.001);


    float3 Kd = 1 - F; // diffuse color 
    Kd = Kd * (1.0 - metallic);
    float3 diff = Kd * albedo / PI;

    uint slidenum = 0;

    for (uint s = 0; s < directionlight.sliceCount; ++s)
    {
        if (depth > directionlight.sliceBoxMaxandZ[s].w)
            ++slidenum;
    }

    float4x4 lightproj = mul(directionlight.lightSliceProj[slidenum], directionlight.lightView);
    
    //float4 lightwcoord = mul(directionlight.lightView, wpos);
    float4 shadowcoord = mul(lightproj, wpos);


    shadowcoord /= shadowcoord.w;
    shadowcoord.xy = shadowcoord.xy * 0.5 + 0.5;
    shadowcoord.y = 1 - shadowcoord.y;
    float pixdepth = shadowcoord.z;
    shadowcoord.z = slidenum;
    float3 vShadowMapDims; // need to keep in sync with .cpp file
    uint ele, level;
    ShadowMap.GetDimensions(0, vShadowMapDims.x, vShadowMapDims.y, ele, level);
    
    float2 texsize = 1.0f / float2(vShadowMapDims.x, vShadowMapDims.y);


    float test = 1.0f;


    float2 disc = Silhouette[pos].rg;


    float2 normalizedCoordinate = float2(0.0f,0.0f);
    
    float2 xyClip = float2(0.0f, 0.0f);

    float2 left = float2(0.0f, 0.0);
    float2 right = float2(0.0f, 0.0);
    float2 up = float2(0.0f, 0.0);
    float2 down = float2(0.0f, 0.0);
    float4 subCoord = float4(0.0, 0.0, 0.0, 0.0);
    float interres = 0.0f;
    if (dot(disc, 1.0) > 0.0f)  // only care about the edge
    {
        float2 edgedir = (disc - 0.75f) * 4.0f; // the direction that is going to interpolation
    
    //directionlight.
        left = searchend(slidenum, wpos, float2(vShadowMapDims.x, vShadowMapDims.y), lightproj, directionlight.lightSliceProjtoViewProj[slidenum], uv, disc, int2(-1, 0));
        right = searchend(slidenum, wpos, float2(vShadowMapDims.x, vShadowMapDims.y), lightproj, directionlight.lightSliceProjtoViewProj[slidenum], uv, disc, int2(1, 0));
        up = searchend(slidenum, wpos, float2(vShadowMapDims.x, vShadowMapDims.y), lightproj, directionlight.lightSliceProjtoViewProj[slidenum], uv, disc, int2(0, -1));
        down = searchend(slidenum, wpos, float2(vShadowMapDims.x, vShadowMapDims.y), lightproj, directionlight.lightSliceProjtoViewProj[slidenum], uv, disc, int2(0, 1));

        subCoord = frac(shadowcoord * float4(vShadowMapDims.x, vShadowMapDims.y, 1.0f, 1.0f));



     
        //normalizedCoordinate.x = InterpolationPos(float2(left.x, right.x), edgedir.x, subCoord.x);
        //normalizedCoordinate.y = InterpolationPos(float2(down.x, up.x), -edgedir.y, subCoord.y);

        normalizedCoordinate.x = normalizecoord(left.x, right.x, subCoord.x);
        normalizedCoordinate.y = normalizecoord(up.x, down.x, subCoord.y);

        //xyClip.x = lerp(step(subCoord.y, normalizedCoordinate.x), step(1.0f - normalizedCoordinate.x, subCoord.y), step(edgedir.y, 0.0f));
        //xyClip.y = lerp(step(1.0f - normalizedCoordinate.y, subCoord.x), step(1.0f - normalizedCoordinate.y, 1.0f - subCoord.x), step(edgedir.x, 0.0f));
     //   fill = dot(xyClip, 1.0f); // sum
        float xres = step(0.0, normalizedCoordinate.x - normalizedCoordinate.y);
        float yres = step(0.0, normalizedCoordinate.x -1 + normalizedCoordinate.y);


        interres = refillshadow(normalizedCoordinate, uint4(left.y, right.y, up.y, down.y)); //(left,right,up,down)

        //normalizedCoordinate.x = xres;
        //normalizedCoordinate.y = yres;

        //interres = saturate(yres);
       
        //interres = (left.y + right.y)*0.5f;
  //      test = dot(xyClip, 1.0f); // sum
		
		//// If discontinuity is in both directions on a single axis, fill.
  //      test = lerp(1.0f, test, abs(edgedir.x));
  //      test = lerp(1.0f, test, abs(edgedir.y));
  //      test = saturate(test);

    }

    interres = 1.0 - interres;


//    int looksize = 3;
//   // looksize = clamp(looksize, 1, 3);


    


//    float total = 0.0f;
////    [unroll]
//    for (int x = -looksize; x <= looksize; ++x)
//    {
//  //      [unroll]
//        for (int y = -looksize; y <= looksize; ++y)
//        {
//            float lightdepth = ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz + float3(texsize * float2(x, y), 0.0f), 0);
//            if ((pixdepth) > lightdepth+0.005f)
//                test += 1.0f;
//        }

//    }
//    test /= (looksize * 2 + 1) * (looksize * 2 + 1);
//   // test /= total;
//    test = 1.0f - test;


        float2 vTexelUnits = 1.0f / vShadowMapDims;





   

    if ((pixdepth) > ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz, 0).r + 0.003f)
        test = 0.0f;


    if (directionlight.debugmask & SMSRUSED)
        test = test * interres;
////    test = exp(-120.0f * pixdepth) * exp(120.0f * ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz, 0).r);

//    test = saturate(test);
////    test = exp(-80 * pixdepth) * ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz, 0).r;



    
    float3 final = (diff + spec) * NL * directionlight.lightColor.xyz * directionlight.lightIntensity * test + albedo*0.04;
  
    //if (shadowcoord.z>0)
    //HDR[pos] = test;
    //else
  //      HDR[pos] = float4(final, 1.0);
    //if ((shadowcoord.z) > 0.0f)
    //    HDR[pos] = float4(1.0f, 0.0f, 0.0f, 0.0f);
   // else if (shadowcoord.xy>1.0)
  //  else
    //    HDR[pos] = float4(ShadowMap.SampleLevel(pointsampler, uv, 0).r.xxxx);
    //HDR[pos] = float4(ShadowMap.SampleLevel(pointsampler,shadowcoord.xy, 0).r.xxxx);
  //  if (lightwcoord.z<0.0)
   //    HDR[pos] = float4(-shadowcoord.z.rrrr);
 //   HDR[pos] = float4(ShadowMap.SampleLevel(pointsampler, shadowcoord.xy, 0).r.xxxx);

    if (directionlight.debugmask & CASCADEDCOLOR)
    {
        final += (debugcolor[slidenum] * 0.5f).rgb;
    }
    HDR[pos] = float4(final, 1.0)/* + debugcolor[slidenum] * 0.5f*/;
 //   if ((pixdepth) > ShadowMap.SampleLevel(pointsampler, shadowcoord.xyz, 0).r + 0.005f)
  //  HDR[pos] = float4(final, 1.0);
    //if (shadowcoord.x >= 0 && shadowcoord.x <= 1 && shadowcoord.y >= 0 && shadowcoord.y <= 1)
    //    HDR[pos] = float4(final, 1.0) + float4(shadowcoord.xy, 0.0, 0.0) * 0.2 * (1 + slidenum);
 //  float testleft = 

//    HDR[pos] = (dot(disc, 1.0) == 0.0f) * float4(final, 0.0) + float4(left.x, right.x, up.x, down.x);
//    HDR[pos] = (dot(disc, 1.0) == 0.0f) * float4(final, 0.0) + float4(up.y, down.y, 0.0f, 0.0f) +debugcolor[slidenum] * 0.3f;


  //  HDR[pos] = float4(final, 0.0) + debugcolor[slidenum] * 0.3f;

  //  HDR[pos] = (dot(disc, 1.0) == 0.0f) * float4(final, 0.0) + float4(right, 0.0, 0.0);

//    HDR[pos] = (dot(disc, 1.0) == 0.0f) * float4(final, 0.0) + float4(normalizedCoordinate, 0.0, 0.0);
 //   HDR[pos] = (dot(disc, 1.0) == 0.0f) * float4(final, 1.0) + float4(disc, 0.0, 0.0);
   // HDR[pos] = (dot(disc, 1.0) == 0.0f) * float4(final, 0.0)+subCoord;

}