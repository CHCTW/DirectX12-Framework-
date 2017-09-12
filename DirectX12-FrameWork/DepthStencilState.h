#pragma once
#include "stdafx.h"
class DepthStencilState
{
public:
	DepthStencilState()
	{
		mDepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		mDepthStencilState.DepthEnable = FALSE;
		mDepthStencilState.StencilEnable = FALSE;
		mDepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	}
	DepthStencilState(bool depthtest)
	{
		mDepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		mDepthStencilState.DepthEnable = depthtest;
		mDepthStencilState.StencilEnable = FALSE;
		mDepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	}

	CD3DX12_DEPTH_STENCIL_DESC mDepthStencilState;
};