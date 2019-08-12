#pragma once
#include "d3dx12.h"
#include <vector>
#include "stdafx.h"
class GPUResource;

class Sampler;


struct RootParameter
{
	RootParameter():mType(PARAMETERTYPE_UNDEFINE),mTable(true), mResCounts(1), mResource(nullptr), rangeflag(D3D12_DESCRIPTOR_RANGE_FLAG_NONE), mVisibility(D3D12_SHADER_VISIBILITY_ALL),
		mShaderSpace(0), mUAVMipLevel(0)
	{

	}
	D3D12_SHADER_VISIBILITY mVisibility;
	ParameterType mType;
	bool mTable;
	UINT mBindSlot;
	UINT mShaderSpace;
	UINT mResCounts;
	UINT mUAVMipLevel;
	union  //since they both use none = 0 , we only need set  0 for flag;
	{
		D3D12_DESCRIPTOR_RANGE_FLAGS rangeflag;
		D3D12_ROOT_DESCRIPTOR_FLAGS descflag;
	};
	union
	{
		GPUResource* mResource;
		Sampler* mSampler;
		const void *mConstantData;
	};


};

class Render;
class RootSignature
{
public:
	
	RootSignature();
	bool initialize(ID3D12Device* device);
	void realease();
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC mRootSignatureDesc;
	ID3D12RootSignature* mRootSignature;
	std::vector<RootParameter> mParameters;
};