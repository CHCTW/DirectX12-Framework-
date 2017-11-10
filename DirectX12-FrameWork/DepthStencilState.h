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
	DepthStencilState(bool depthtest,bool depthwrite  = true)
	{
		mDepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		mDepthStencilState.DepthEnable = depthtest;
		mDepthStencilState.StencilEnable = FALSE;
		mDepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		if (!depthwrite)
			mDepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	}

	CD3DX12_DEPTH_STENCIL_DESC mDepthStencilState;
};