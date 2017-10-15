#pragma once
#include "stdafx.h"
class RasterizerState
{
public:
	RasterizerState(D3D12_CULL_MODE state = D3D12_CULL_MODE_NONE, D3D12_FILL_MODE fill = D3D12_FILL_MODE_SOLID)
	{
		mRasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		mRasterizerState.CullMode = state;
		mRasterizerState.FillMode = fill;
	}
	D3D12_RASTERIZER_DESC mRasterizerState;
};