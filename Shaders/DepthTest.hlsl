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
cbuffer SceneConstantBuffer : register(b0)
{
	float offset;
};
struct PSInput
{
	float4 position : SV_POSITION;
	float3 color : P_COLOR;
};
PSInput VSMain(float3 position : POSITION, float3 color : COLOR)
{
	PSInput result;
	position.x = position.x + offset;
	result.position = float4(position,1.0);
	result.color = float3(position.z, position.z, position.z);

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	//return float4(input.uv,0.0f,0.0f);
	return float4(input.color,1.0);
}
