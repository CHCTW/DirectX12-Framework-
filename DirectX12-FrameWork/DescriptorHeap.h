#pragma once
#include "d3dx12.h"
#include "stdafx.h"
#include <iostream>
using namespace std;


class Render;

class DescriptorHeap
{
public:
	DescriptorHeap();
	bool ininitialize(ID3D12Device* device,UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	Handles addResource(ViewType type,ID3D12Resource* res, void* des, ID3D12Resource* countRes = nullptr);
	Handles addSampler(D3D12_SAMPLER_DESC& samplerdes);
	void release();
	D3D12_DESCRIPTOR_HEAP_DESC mDescHeapDesc;
	ID3D12DescriptorHeap* mDescHeap;
	UINT mDescSize;
	union
	{
		struct CBV_SRV_UAV_Index
		{
			UINT mCBVEndIndex;
			UINT mSRVEndIndex;
			UINT mUAVEndIndex;
		} cbv_srv_uav_index;
		UINT mEndIndex;
	};
	union
	{
		struct CBV_SRV_UAV_Handle
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE mCBVEndHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE mSRVEndHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE mUAVEndHandle;
		} cbv_srv_uav_handle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE  mHeapEndHandle;
	};


private:
	ID3D12Device* mDevice;
};

