#pragma once
#include "stdafx.h"
#include "d3dx12.h"
#include <vector>
using namespace std;
class Resource
{
public:
	D3D12_RESOURCE_STATES mState;
	ID3D12Resource* mResource;
	ID3D12Resource* mUploadBuffer;
	Handles mSRV;
	vector<Handles> mUAV;
	Handles mCBV;
	// sitll gonna keep here due to cmdlist bindding, need to think more, but it is going to be use a arrays 
	vector<Handles> mRTV;
	vector<Handles> mDSV;
	//*********************************************************//

	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;
	void* CpuMapAddress;
	Resource::Resource():mResource(nullptr),mUploadBuffer(nullptr), CpuMapAddress(nullptr)
	{

	}
	void release();
};