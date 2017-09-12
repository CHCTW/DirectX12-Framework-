#pragma once
#include "stdafx.h"
class BlendState
{
public:
	BlendState::BlendState()
	{
		mBlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	}
	BlendState::BlendState(bool colorblend)
	{
		mBlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		mBlendState.RenderTarget[0].BlendEnable = true;
		mBlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	}
	D3D12_BLEND_DESC mBlendState;
};