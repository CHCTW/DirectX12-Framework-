#include "../GraphicsUtility/Lightfunctions.hlsl"
#include "../GraphicsUtility/GraphicsUtility.hlsl"
#define GROUPSIZE 16
#define MAXITERATION 40.0f
#define MAXHIZLEVEL 10
cbuffer CameraBuffer : register(b0)
{
    Camera camera;
};
struct TraceConstants
{
    float zThickness;
    float stride;
    float maxStep;
    float maxDistance;
    float strideZCutoff;

};

cbuffer TraceConstBuffer : register(b1)
{
    TraceConstants traceConstants;
};


Texture2D NormalTexture : register(t0);
Texture2D DepthTexture : register(t1);
Texture2D<float2> HiZTexture : register(t2);
Texture2D HDRTexture : register(t3);
RWTexture2D<float4> TraceUV : register(u0);
sampler pointsample : register(s0);
float fechHiZ(float2 uv,uint mip,float tracestate)
{
  

    // tracestate describe the trace z negative or postive, if trace in negative, we use minZ, if trace positive , we use maxZ
    float2 hiz = HiZTexture.SampleLevel(pointsample, uv, mip).rg;
    float res;
    if(tracestate>=0.0f)
        res = hiz.r;
    else
        res = hiz.g;
     // in view space, we look to negative z, that's why need to mul -1, mul back just denormalized the value
    return res * camera.back * (-1.0f);

}
bool interscet(float2 uv,uint mip,float tracestate,float z)
{
    float2 hizs = HiZTexture.SampleLevel(pointsample, uv, mip).rg;
    float channelchoose;
    if (tracestate >= 0.0f)
        channelchoose = hizs.r;
    else
        channelchoose = hizs.g;
     // in view space, we look to negative z, that's why need to mul -1, mul back just denormalized the value
    float hiz = channelchoose * camera.back * -1.0f;
    if ((hiz - z) >= 0.0f)
    {
        if (mip == 0)
        {
            if ((hiz - z) <= 5.0f)
                return true;
            return false;
        }
        return true;
    }
    return false;
}
float4 raymove(float4 PZK, float4 dPZK, uint level)
{

    uint maxlevel = 5;
    uint2 dim;
    float2 fdim;
    DepthTexture.GetDimensions(level, dim.x, dim.y, maxlevel);
    fdim = dim;
    float4 movestep = dPZK * pow(2, level); // higher level can move more, still in uv space
    //float2 aligned_uv = floor(input.uv * cell_count_at_start) / cell_count_at_start + 0.25 / cell_count_at_start;

    return PZK + movestep;
}
/*float4 raymovefromori(float4 PZK, float4 dPZK, uint level)
{

}*/
float traceIntersection(int maxlevel,float2 fdim, float2 startpix, float3 startpos, float3 dir, float jitter, out float2 hitpix, out float3 hitpos)
{
    hitpos = float3(0.0f, 0.0f, 0.0f);
    hitpix = float2(0.0f, 0.0f);
    //hitpix = float2(0.0f, 0.0f);



    float raylength = ((startpos.z + dir.z * traceConstants.maxDistance) < -camera.front) ? (-camera.front - startpos.z) / abs(dir.z) : traceConstants.maxDistance;


    //raylength = 200;
        float3 endpos = startpos + dir * raylength;
    float4 endndc = mul(camera.proj, float4(endpos, 1.0f));
    float tracestate = endpos.z - startpos.z;
    // dir.z >=0 means trace toward to the carmear near plane, else trace toward far plane
    if (dir.z >= 0.0f)
        tracestate = 1.0f;
    else
        tracestate = -1.0f;
    //endndc /= endndc.w;
    float4 startndc = mul(camera.proj, float4(startpos, 1.0f));
    //startndc /= startndc.w;
    
    // fot screen space interpolation, all value should /w first , also 1/w is interpolable in screen space, 
    float k0 = 1.0f / startndc.w;
    float k1 = 1.0f / endndc.w;

    float3 q0 = startpos * k0;
    float3 q1 = endpos * k1;
    


    // to uv space
    startndc.xy *= k0;
    startndc.xy = startndc.xy * 0.5 + 0.5;
    startndc.y = 1.0f - startndc.y;



    endndc.xy *= k1;
    // ndc to texture space
    endndc.xy = endndc.xy * 0.5 + 0.5;
    endndc.y = 1.0f - endndc.y;



    float2 endpix = endndc.xy * fdim;
    
    
    if (all(startndc.xy == endndc.xy))
        return 0.0f;
    //endpix += ((startndc.xy == endndc.xy) ? float2(1.0f, 1.0f) : float2(0.0f, 0.0f));


    float2 delta;
    bool permute = false;
    // total uv movement
    delta = endndc.xy - startndc.xy;
    // total pixel movement
    float2 deltapix = endpix - startpix;


    float deltak = k1 - k0; // proj depth
    float deltaqz = q1.z - q0.z; // linear depth in view space
  


    // we need the change of uv pos , z/w,  1/w, alogn the axis that can describe the line more detail
    float4 dPZK = float4(delta, deltaqz, deltak);

    // what can we do in uv?? uv is normalized, that is don't care about real pixel offset, when / different level of dimension 
    float4 DeltaPZK = float4(delta, deltaqz, deltak);


    // get the uv coord, z/w, 1/w , in the start point
  //  startndc.xy = (floor(startndc.xy * fdim) + float2(0.5f, 0.5f)) / fdim;
    float4 PZK = float4(startndc.xy + float2(0.5f, 0.5f) / fdim, q0.z, k0);
    

    float2 tracedir = float2(0.0f, 0.0f);
    float2 tracesign;
    if (abs(delta.x) < abs(delta.y))
    {
        // This is a more-vertical line
        permute = true;
        // divide by the total pixel move in y - axis, that is we have delta/(dp), (z/w)/dp, (1/w)/dp
        dPZK /= abs(deltapix.y);
        tracedir = float2(0.0f, deltapix.y / abs(deltapix.y));

    }
    else
    {
        dPZK /= abs(deltapix.x);
        tracedir = float2(deltapix.x / abs(deltapix.x), 0.0f);
    }
       


        // dpzk in differnt level
    float4 dPZKs[MAXHIZLEVEL];
    float2 fdims[MAXHIZLEVEL];
    float2 finvdims[MAXHIZLEVEL];

    for (int level = 0; level <= maxlevel; ++level)
    {
        
        //uint2 dim;
        uint max;
        HiZTexture.GetDimensions(level, fdims[level].x, fdims[level].y, max);
        //fdims[level] = dim;
        finvdims[level] = 1.0f / fdims[level];
        // trace pixel 
        float2 endpixlevel = endndc.xy * fdims[level];
        float2 startpixlevel = startndc.xy * fdims[level];
        endpixlevel += ((floor(startpixlevel.xy) == floor(endpixlevel.xy)) ? float2(1.0f, 1.0f) : float2(0.0f, 0.0f));
        float2 deltapixlevel = endpixlevel - startpixlevel;
        dPZKs[level] = DeltaPZK;
        if (permute)
        {
            dPZKs[level] /= abs(deltapixlevel.y);
        }
        else
            dPZKs[level] /= abs(deltapixlevel.x);
    }


    PZK += dPZKs[0] * jitter * 2.0f;



    int miplevel = 0;
    float step = 0.0f;
    // start to trace
    // this part can be change if the hi-z has enough layer into like 1-pix, even with out boundary check
    // temporary paramerter, will chagne later
    float4 next = float4(0, 0, 0, 0);
  /*  while (miplevel > -1 && step < MAXITERATION && all(PZK.xy >= 0.0f) && all(PZK.xy <= 1.0f))
    {

        next = raymove(PZK, dPZK, miplevel);

        float hiz = fechHiZ(next.xy, miplevel,tracestate);
        // hiz is already in view space  z, which mean is negative
        float linez = next.z / next.w;
        // sinze z is negtive  linez<=z means intersection happened
        if ((hiz-linez) > 0.0f)
        {
            --miplevel;// get to more fine texture
        }
        else // doesn't have intersection, just go next
        {
            
            //if (miplevel == maxlevel)
                PZK = next;
            // go more rough texture
            miplevel = clamp(miplevel+1, 0, maxlevel);
        }
      


        ++step;
    }*/







   float4 tempray;
    float degoffset;
    int2 oldpix = startpix; // initial trace pixel
    int2 newpix = startpix;
    float2 alignuv;
    bool check;
    while (miplevel > -1 && step < MAXITERATION && all(PZK.xy >= 0.0f) && all(PZK.xy <= 1.0f))
    {
        // alignuv will keep uv in the center of the pixel, - a small value to prevent croos to next pixel, since most time this uv is actually really close the boundry     
        alignuv = (floor(PZK.xy * fdims[miplevel] - tracedir * float2(0.1f, 0.1f)) + float2(0.5f, 0.5f)) * finvdims[miplevel];
        degoffset = ((alignuv - PZK.xy) * tracedir).x + ((alignuv - PZK.xy) * tracedir).y; // use tracedir to keep the direction we care
        next = PZK + (-degoffset + 1.5) * dPZKs[miplevel]; // move to end of the next pixel
        float2 fetchuv = PZK.xy + (-degoffset + 1.0) * dPZKs[miplevel].xy; // move center of the next pixel, use this to fetch z 
       


        check = interscet(fetchuv, miplevel, tracestate, next.z / next.w);


        if (check) // intersect, we don't move use more detail depth texture 
        {
            --miplevel;
        }
        else // no intersection 
        {
            PZK = next; // always move to next position
            int tempnextlevel = clamp(miplevel + 1, 0, maxlevel); // temp next level 
            oldpix = PZK.xy * fdims[tempnextlevel] - tracedir * float2(0.01f, 0.01f);
            newpix = fetchuv * fdims[tempnextlevel]; // fetch uv is at the cetner of pix no need to offset 
            if (all(oldpix == newpix)) // doesn't cross pixel in next level, can add pixlevel, if we cross, we don't add level
                miplevel = tempnextlevel;
        }
        ++step;
    }



  

    hitpos = endpos;
    hitpos.z = endpos.z;
    hitpix = next.xy;
 //   hitpix = endndc.xy;


  //  hitpix = float2(0.0f,0.0f);
 //   if(step==1)
//    return 1.0f;

//    return float(step) / float(MAXITERATION);

   // hitpix.x = float(step) / MAXITERATION;
   // hitpix.y = float(step) / MAXITERATION;
 //   hitpix.xy = dPZKs[5].xy;
    //return false;
    //hitpix = clamp(hitpix, float2(0.0, 0.0), float2(1.0, 1.0));
        
   //return false;
    // <1 means find
    if (hitpix.x > 1.0f || hitpix.x < 0.0f || hitpix.y > 1.0f || hitpix.y < 0.0f || !check)
    {
        return 0.0f;
    }


    if(miplevel<0)
    {
    //    return true;
        float face = dot(decode(NormalTexture.SampleLevel(pointsample, next.xy, 0).rg), dir);
        if (face<=0)
            return 1.0f;
        return 0.0f;
    }
    return 0.0f;



}


[numthreads(GROUPSIZE, GROUPSIZE, 1)] // block size 256 pixel, 
void CSMain(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint2 pos = id.xy;
    uint2 dim;
    DepthTexture.GetDimensions(dim.x, dim.y);
    

    float depth = DepthTexture[pos].r;
    [branch]
    if(all(pos<dim)) // really have data in this pixel
    {
       
        float2 uv = pos;
        uv = (uv) / float2(dim);
        float4 projcoord;
        projcoord.xy = uv;
        projcoord.y = 1.0 - projcoord.y;
        projcoord.xy = projcoord.xy * 2.0f - 1.0f;
        projcoord.z = depth;
        projcoord.w = 1.0f;
        float4 vpos = mul(camera.projinverse, projcoord);
        vpos.xyzw /= vpos.w;
        float3 normal = decode(NormalTexture[pos].rg);
        float3 viewdir = normalize(vpos.xyz);
        float3 reflectdir = normalize(2 * dot(-viewdir, normal) * normal + viewdir);
        float RV = dot(reflectdir, viewdir);
        float jitter = traceConstants.stride >= 1.0f ? float(int(pos.x + pos.y) & 1) * 0.5f : 0.0f;

        // trace in view space
     //   HiZTexture.Load(pos,)

        float2 hitpix = float2(0, 0);
        float3 hitpos = float3(0, 0, 0);



        int maxlevel = 5;
        float inter = 0.0f;
        [branch]
        if(depth<1.0f)
            inter = traceIntersection(maxlevel,dim, pos, vpos.xyz, reflectdir, 1.0f, hitpix, hitpos);
        // store view space depth
        float hitdepth = HiZTexture.SampleLevel(pointsample, hitpix, 0).r * camera.back * (-1.0f);
        //if(inter)
        //    TraceUV[pos] = HDRTexture.SampleLevel(pointsample, hitpix, 0);
        //else
        //TraceUV[pos] = float4(reflectdir, 0.0);
        TraceUV[pos] = float4(hitpix.xy, hitdepth, RV) * inter;


    }
}