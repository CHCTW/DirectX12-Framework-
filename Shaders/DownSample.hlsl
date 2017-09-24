Texture2D<float4> SrcTexture : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
[numthreads(8, 8, 1)]
void GenerateMipMaps(uint3 DTid : SV_DispatchThreadID)
{

    uint2 dim;
    DstTexture.GetDimensions(dim.x, dim.y);
    // get dimension 

    float2 texcoords = (1 / (float)dim) * (DTid.xy + 0.5);

 
    //The samplers linear interpolation will mix the four pixel values to the new pixels color
    float4 color = SrcTexture.SampleLevel(BilinearClamp, texcoords, 0);

    //Write the final color into the destination texture.
    DstTexture[DTid.xy] = color;
}