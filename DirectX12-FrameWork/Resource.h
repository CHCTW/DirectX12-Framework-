#pragma once
#include "stdafx.h"
#include "d3dx12.h"
class Resource
{
public:
	D3D12_RESOURCE_STATES mState;
	ID3D12Resource* mResource;
	ID3D12Resource* mUploadBuffer;
	Handles mSRV;
	Handles mUAV;
	Handles mCBV;
	// going move these to texture class, also make it array so it can handle lod rendertarget 
	Handles mRTV;
	Handles mDSV;
	//*********************************************************//

	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;
	void* CpuMapAddress;
	Resource::Resource():mResource(nullptr),mUploadBuffer(nullptr), CpuMapAddress(nullptr)
	{

	}
	void release();
};