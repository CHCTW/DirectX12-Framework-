#include "RootSignature.h"
#include "stdafx.h"
#
RootSignature::RootSignature():mRootSignature(NULL)
{

}

bool RootSignature::initialize(ID3D12Device* device)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
	// build mdx12parameters array here



	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;
	CD3DX12_ROOT_PARAMETER1 dx12param;
	std::vector<CD3DX12_ROOT_PARAMETER1> mDX12RootParameters;
	ranges.resize(mParameters.size());
	for (int i = 0 ; i < mParameters.size() ; ++i)
	{
		if (mParameters[i].mType == PARAMETERTYPE_UNDEFINE)
			return false;
		switch (mParameters[i].mType)
		{
		case PARAMETERTYPE_ROOTCONSTANT:
			dx12param.InitAsConstants(mParameters[i].mResCounts, mParameters[i].mBindSlot, mParameters[i].mShaderSpace, mParameters[i].mVisibility);
			break;
		case PARAMETERTYPE_CBV:
			if (mParameters[i].mTable)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, mParameters[i].mResCounts, mParameters[i].mBindSlot, mParameters[i].mShaderSpace, mParameters[i].rangeflag);
				dx12param.InitAsDescriptorTable(1, &ranges[i], mParameters[i].mVisibility);
			}
			else
				dx12param.InitAsConstantBufferView(mParameters[i].mBindSlot, mParameters[i].mShaderSpace, mParameters[i].descflag, mParameters[i].mVisibility);
			break;
		case PARAMETERTYPE_SRV:
			if (mParameters[i].mTable)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, mParameters[i].mResCounts, mParameters[i].mBindSlot, mParameters[i].mShaderSpace, mParameters[i].rangeflag);
				dx12param.InitAsDescriptorTable(1, &ranges[i], mParameters[i].mVisibility);
			}
			else
				dx12param.InitAsShaderResourceView(mParameters[i].mBindSlot, mParameters[i].mShaderSpace, mParameters[i].descflag, mParameters[i].mVisibility);
			break;
		case PARAMETERTYPE_UAV:
			if (mParameters[i].mTable)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, mParameters[i].mResCounts, mParameters[i].mBindSlot, mParameters[i].mShaderSpace, mParameters[i].rangeflag);
				dx12param.InitAsDescriptorTable(1, &ranges[i], mParameters[i].mVisibility);
			}
			else
				dx12param.InitAsUnorderedAccessView(mParameters[i].mBindSlot, mParameters[i].mShaderSpace, mParameters[i].descflag, mParameters[i].mVisibility);
			break;
		case PARAMETERTYPE_SAMPLER:
			ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, mParameters[i].mResCounts, mParameters[i].mBindSlot, mParameters[i].mShaderSpace, mParameters[i].rangeflag);
			dx12param.InitAsDescriptorTable(1, &ranges[i], mParameters[i].mVisibility);
			break;
		default:

			break;
		}
		mDX12RootParameters.push_back(dx12param);
	}

	mRootSignatureDesc.Init_1_1(mDX12RootParameters.size(), mDX12RootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ID3DBlob* signature;
	ID3DBlob* error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&mRootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
	return true;
}

void RootSignature::realease()
{
	SAFE_RELEASE(mRootSignature);
}