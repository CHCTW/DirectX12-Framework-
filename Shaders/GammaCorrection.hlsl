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
PSInput VSMain(float2 position : POSITION, float2 uv : TEXCOORD, uint id : SV_VertexID)
{
	PSInput result;
	result.uv = float2((id << 1) & 2, id & 2);
	result.position = float4(result.uv  * float2(2, -2) + float2(-1, 1), 0, 1);


	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
//	return float4(input.uv,0.0f,0.0f);

  //  int tempx = input.uv.x * 20;
    
	float x = 0.5f;
    if (input.uv.x <= 0.25 || input.uv.x >= 0.75)
    {
        int ix = input.uv.y * 900;
        x = (ix % 2);
    }
	else if (input.uv.y >= 0.5)
	{
		//x = x / (1 + x);
     
		x = pow(x, 1.0/2.2f);
	//	x = 0.1;
	}
	return float4(x,x,x,x);
}
