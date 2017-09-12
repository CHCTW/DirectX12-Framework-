#pragma once
#include "stdafx.h"
class RasterizerState
{
public:
	RasterizerState()
	{
		mRasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		mRasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	}
	D3D12_RASTERIZER_DESC mRasterizerState;
};