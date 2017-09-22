#pragma once
#include "d3dx12.h"
#include "stdafx.h"
class DescriptorHeap;
class Sampler
{
public:
	Sampler(D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,  // due to pass too many parameters, should use inline to prevent a lot of copy
		D3D12_TEXTURE_ADDRESS_MODE u = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE v = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE w = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		FLOAT bordercolor[4] = DefaultBorderColor,
		FLOAT miplodbias = 0,
		UINT maxanisotropy = 0,
		FLOAT minlod = 0.0f,
		FLOAT maxlod = D3D12_FLOAT32_MAX,
		D3D12_COMPARISON_FUNC comparisonfunc = D3D12_COMPARISON_FUNC_NEVER
	) 
	{
		mSamplerDesc.Filter = filter;
		mSamplerDesc.AddressU = u;
		mSamplerDesc.AddressV = v;
		mSamplerDesc.AddressW = w;
		mSamplerDesc.BorderColor[0] = bordercolor[0];
		mSamplerDesc.BorderColor[1] = bordercolor[1];
		mSamplerDesc.BorderColor[2] = bordercolor[2];
		mSamplerDesc.BorderColor[3] = bordercolor[3];
		mSamplerDesc.MipLODBias = miplodbias;
		mSamplerDesc.MaxAnisotropy = maxanisotropy;
		mSamplerDesc.MinLOD = minlod;
		mSamplerDesc.MaxLOD = maxlod;
		mSamplerDesc.ComparisonFunc = comparisonfunc;
	}
	bool createSampler(DescriptorHeap & heap);
	D3D12_SAMPLER_DESC mSamplerDesc;
	Handles mSampler;
};