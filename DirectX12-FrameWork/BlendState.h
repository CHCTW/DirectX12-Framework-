#pragma once
#include "stdafx.h"
class BlendState
{
public:
	BlendState()
	{
		mBlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	}
	BlendState(bool colorblend)
	{
		mBlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		mBlendState.RenderTarget[0].BlendEnable = true;
		mBlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		mBlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		mBlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	}
	D3D12_BLEND_DESC mBlendState;
};