//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);
PSInput VSMain(float2 position : POSITION, float2 uv : TEXCOORD, uint id : SV_VertexID)
{
	PSInput result;
	result.uv = float2((id << 1) & 2, id & 2);
	result.position = float4(result.uv  * float2(2, -2) + float2(-1, 1), 0, 1);


	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	//return float4(input.uv,0.0f,0.0f);
	return g_texture.Sample(g_sampler, input.uv);
}
