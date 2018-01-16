#include "DescriptorHeap.h"
#include "StructureHeaders.h"
#include "stdafx.h"
#include <iostream>
DescriptorHeap::DescriptorHeap():mDescSize(0), mDescHeap(NULL), mEndIndex(0), mDevice(NULL)
{
	mDescHeapDesc = {};
	//cbv_srv_uav_handle.mCBVEndHandle
}

void DescriptorHeap::release()
{
	SAFE_RELEASE(mDescHeap);
}
bool DescriptorHeap::ininitialize(ID3D12Device* device, UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type)
{


	mDescHeapDesc.Type = type;
	mDescHeapDesc.NumDescriptors = HeapSizes[type];
	mDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		mDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;


	HRESULT hr = device->CreateDescriptorHeap(&mDescHeapDesc, IID_PPV_ARGS(&mDescHeap));
	if (!SUCCEEDED(hr))
	{
		std::cout << "Fail to create Descriptor Heap" << std::endl;
		return false;
	}
	mDevice = device;
	mHeapEndHandle = mDescHeap->GetCPUDescriptorHandleForHeapStart();
	mEndIndex = 0;
	mDescSize = device->GetDescriptorHandleIncrementSize(mDescHeapDesc.Type);
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		cbv_srv_uav_handle.mCBVEndHandle = mHeapEndHandle;
		cbv_srv_uav_handle.mCBVEndHandle.Offset(HeapOffset[CBV], mDescSize);
		cbv_srv_uav_index.mCBVEndIndex = HeapOffset[CBV];
		cbv_srv_uav_handle.mSRVEndHandle = mHeapEndHandle;
		cbv_srv_uav_handle.mSRVEndHandle.Offset(HeapOffset[SRV], mDescSize);
		cbv_srv_uav_index.mSRVEndIndex = HeapOffset[SRV];
		cbv_srv_uav_handle.mUAVEndHandle = mHeapEndHandle;
		cbv_srv_uav_handle.mUAVEndHandle.Offset(HeapOffset[UAV], mDescSize);
		cbv_srv_uav_index.mUAVEndIndex = HeapOffset[UAV];

		
	}



	return true;
}
Handles DescriptorHeap::addResource(ViewType type,ID3D12Resource* res, void* des, ID3D12Resource* countRes)
{

	Handles handle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE view(mDescHeap->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE temp;
	switch (type)
	{
	case CBV:
#if defined( _DEBUG )
		if (mDescHeapDesc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			cout << "SRV can only register in CBV_SRV_UAV Heap" << endl;
#endif
		mDevice->CreateConstantBufferView((D3D12_CONSTANT_BUFFER_VIEW_DESC *)des, cbv_srv_uav_handle.mCBVEndHandle);
		view.Offset(cbv_srv_uav_index.mCBVEndIndex, mDescSize);
		handle.Cpu = cbv_srv_uav_handle.mCBVEndHandle;
		handle.Gpu = view;
		cbv_srv_uav_handle.mCBVEndHandle.Offset(1, mDescSize);
		++cbv_srv_uav_index.mCBVEndIndex;
		break;
	case SRV:
#if defined( _DEBUG )
		if (mDescHeapDesc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			cout << "SRV can only register in CBV_SRV_UAV Heap" << endl;
#endif
		mDevice->CreateShaderResourceView(res, (D3D12_SHADER_RESOURCE_VIEW_DESC *)des, cbv_srv_uav_handle.mSRVEndHandle);
		view.Offset(cbv_srv_uav_index.mSRVEndIndex, mDescSize);
		handle.Cpu = cbv_srv_uav_handle.mSRVEndHandle;
		handle.Gpu = view;
		cbv_srv_uav_handle.mSRVEndHandle.Offset(1, mDescSize);
		++cbv_srv_uav_index.mSRVEndIndex;
		break;
	case UAV:
#if defined( _DEBUG )
		if (mDescHeapDesc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			cout << "UAV can only register in CBV_SRV_UAV Heap" << endl;
#endif
		mDevice->CreateUnorderedAccessView(res, countRes, (D3D12_UNORDERED_ACCESS_VIEW_DESC *)des, cbv_srv_uav_handle.mUAVEndHandle);
		view.Offset(cbv_srv_uav_index.mUAVEndIndex, mDescSize);
		handle.Cpu = cbv_srv_uav_handle.mUAVEndHandle;
		handle.Gpu = view;
		cbv_srv_uav_handle.mUAVEndHandle.Offset(1, mDescSize);
		++cbv_srv_uav_index.mUAVEndIndex;
		break;
	case RTV:
#if defined( _DEBUG )
		if (mDescHeapDesc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			cout << "RTV can only register in RTV Heap" << endl;
#endif
		mDevice->CreateRenderTargetView(res, (D3D12_RENDER_TARGET_VIEW_DESC *)des, mHeapEndHandle);
		view.Offset(mEndIndex, mDescSize);
		handle.Cpu = mHeapEndHandle;
		handle.Gpu = view;
		mHeapEndHandle.Offset(1, mDescSize);
		++mEndIndex;
		break;
	case DSV:
#if defined( _DEBUG )
		if (mDescHeapDesc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
			cout << "DSV can only register in DSV Heap" << endl;
#endif
		mDevice->CreateDepthStencilView(res, (D3D12_DEPTH_STENCIL_VIEW_DESC  *)des, mHeapEndHandle);
		view.Offset(mEndIndex, mDescSize);
		handle.Cpu = mHeapEndHandle;
		handle.Gpu = view;
		mHeapEndHandle.Offset(1, mDescSize);
		++mEndIndex;
		break;
	default:
		break;
	}

	return handle;
	
}
Handles DescriptorHeap::getHandles(ViewType type, UINT index)
{
	Handles handle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuview(mDescHeap->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuview(mDescHeap->GetCPUDescriptorHandleForHeapStart());
	gpuview.Offset(HeapOffset[type]+index, mDescSize);
	cpuview.Offset(HeapOffset[type]+index, mDescSize);
	handle.Gpu = gpuview;
	handle.Cpu = cpuview;
	return handle;
	
}
Handles DescriptorHeap::addSampler(D3D12_SAMPLER_DESC& samplerdesc)
{
	Handles handle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE sampler(mDescHeap->GetGPUDescriptorHandleForHeapStart());

#if defined( _DEBUG )

	if (mDescHeapDesc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		cout << "Sampler can only register in Sampler Heap" << endl;
#endif
	mDevice->CreateSampler(&samplerdesc, mHeapEndHandle);
	sampler.Offset(mEndIndex, mDescSize);
	handle.Gpu = sampler;
	handle.Cpu = mHeapEndHandle;
	mHeapEndHandle.Offset(1, mDescSize);
	++mEndIndex;
	return handle;
	
}


