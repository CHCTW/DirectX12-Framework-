#include "Buffer.h"
#include "CommandList.h"
#include "stdafx.h"
#include "StructureHeaders.h"
Buffer::Buffer():mType(NONE), mBufferSize(0), mCounter(false)
{

}
//bool Buffer::createBuffer(ID3D12Device* device, UINT buffersize, D3D12_HEAP_TYPE heaptype = D3D12_HEAP_TYPE_DEFAULT)
//{
//	if (mResource)
//	{
//		cout << "Already Create Buffer" << endl;
//		return false;
//	}
//	HRESULT hr = device->CreateCommittedResource(
//		&CD3DX12_HEAP_PROPERTIES(heaptype),
//		D3D12_HEAP_FLAG_NONE,
//		&CD3DX12_RESOURCE_DESC::Buffer(buffersize),
//		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
//		nullptr,
//		IID_PPV_ARGS(&mResource));
//	if (FAILED(hr))
//		return false;
//
//	mType = VERTEX;
//	mState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
//	mHeapType = heaptype;
//	mBufferSize = buffersize;
//	GpuAddress = mResource->GetGPUVirtualAddress();
//
//	if (mHeapType == D3D12_HEAP_TYPE_DEFAULT)  // if the buffer is in defaul area, need another heap for upload
//	{
//		hr = device->CreateCommittedResource(
//			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//			D3D12_HEAP_FLAG_NONE,
//			&CD3DX12_RESOURCE_DESC::Buffer(buffersize),
//			D3D12_RESOURCE_STATE_GENERIC_READ,
//			nullptr,
//			IID_PPV_ARGS(&mUploadBuffer));
//		if (FAILED(hr))
//			return false;
//	}
//
//	return true;
//}
bool Buffer::createVertexBuffer(ID3D12Device* device,UINT buffersize, UINT strideSize, D3D12_HEAP_TYPE heaptype )
{
	
	if (mResource)
	{
		cout << "Already Create Buffer" << endl;
		return false;
	}
	mCounter = false;
	HRESULT hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(heaptype),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(buffersize),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		nullptr,
		IID_PPV_ARGS(&mResource));
	if (FAILED(hr))
		return false;
	mVertexBuffer.BufferLocation = mResource->GetGPUVirtualAddress();
	mVertexBuffer.StrideInBytes = strideSize;
	mVertexBuffer.SizeInBytes = buffersize;
	mType = STRUCTERED;
	mState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	mHeapType = heaptype;
	mBufferSize = buffersize;
	GpuAddress = mResource->GetGPUVirtualAddress();

	if (mHeapType == D3D12_HEAP_TYPE_DEFAULT)  // if the buffer is in defaul area, need another heap for upload
	{
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(buffersize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mUploadBuffer));
		if (FAILED(hr))
			return false;
	}

	return true;
}

bool Buffer::createIndexBuffer(ID3D12Device* device, UINT buffersize, IndexBufferFormat indexformat, D3D12_HEAP_TYPE heaptype)
{
	if (mResource)
	{
		cout << "Already Create Buffer" << endl;
		return false;
	}
	HRESULT hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(heaptype),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(buffersize),
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		nullptr,
		IID_PPV_ARGS(&mResource));
	if (FAILED(hr))
		return false;
	mIndexBuffer.BufferLocation = mResource->GetGPUVirtualAddress();

	if (indexformat == INDEX_BUFFER_FORMAT_32BIT)
		mIndexBuffer.Format = DXGI_FORMAT_R32_UINT;
	else
		mIndexBuffer.Format = DXGI_FORMAT_R16_UINT;
	mIndexBuffer.SizeInBytes = buffersize;
	mType = INDEX;
	mState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	mHeapType = heaptype;
	mBufferSize = buffersize;
	GpuAddress = mResource->GetGPUVirtualAddress();

	if (mHeapType == D3D12_HEAP_TYPE_DEFAULT)  // if the buffer is in defaul area, need another heap for upload
	{
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(buffersize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mUploadBuffer));
		if (FAILED(hr))
			return false;
	}

	return true;
}

bool Buffer::createConstantBuffer(ID3D12Device* device, DescriptorHeap &heap, UINT buffersize)
{
	if (mResource)
	{
		cout << "Already Create Buffer" << endl;
		return false;
	}

	HRESULT hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((buffersize + 255) & ~255),// alignment.... so, need to be don'e like this
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mResource));
	if (FAILED(hr))
		return false;
	mType = CONSTANT;
	mState = D3D12_RESOURCE_STATE_GENERIC_READ;
	mHeapType = D3D12_HEAP_TYPE_UPLOAD;
	GpuAddress = mResource->GetGPUVirtualAddress();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = mResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (buffersize + 255) & ~255;
	mCBV = heap.addResource(CBV, mResource, &cbvDesc);
	mBufferSize = (buffersize + 255) & ~255;
	return true;
}
bool Buffer::createConstantBuffer(ID3D12Device* device, UINT buffersize)
{
	if (mResource)
	{
		cout << "Already Create Buffer" << endl;
		return false;
	}

	HRESULT hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((buffersize + 255) & ~255),// alignment.... so, need to be don'e like this
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mResource));
	if (FAILED(hr))
		return false;
	mType = CONSTANT;
	mState = D3D12_RESOURCE_STATE_GENERIC_READ;
	mHeapType = D3D12_HEAP_TYPE_UPLOAD;
	GpuAddress = mResource->GetGPUVirtualAddress();

	return true;
}


bool Buffer::createStructeredBuffer(ID3D12Device* device, DescriptorHeap &heap, UINT strideSize, UINT elementcount, StructeredBufferType type, bool counter, bool raw, bool padding , D3D12_HEAP_TYPE heaptype)
{
	//if(Buffer)

	
	//if(type == STRUCTERED_BUFFER_TYPE_READ)

	mState = D3D12_RESOURCE_STATE_GENERIC_READ;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
	
	if (type == STRUCTERED_BUFFER_TYPE_READ_WRITE)
	{
		mState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	
	if (padding)
	{
		strideSize = (strideSize + 16)&~16; // structer buffer is better for alignment with 128bit, 16 byte
	}

	
	UINT buffersize = elementcount*strideSize;
	if (counter)
	{
		if(type != STRUCTERED_BUFFER_TYPE_READ_WRITE)
		{
			cout << "Counter Buffer needs to be read write buffer" << endl;
			return false;
		}
		UINT ali = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		buffersize = (buffersize + ali-1) & ~(ali-1); // alignment the whole buffer to 256 bytes
		buffersize += sizeof(UINT);// add counter at the end of buffer
	}
	if (mResource)
	{
		cout << "Already Create Buffer" << endl;
		return false;
	}
	HRESULT hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(heaptype),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(buffersize, flags),
		mState,
		nullptr,
		IID_PPV_ARGS(&mResource));
	if (FAILED(hr))
		return false;
	

	mType = STRUCTERED;
	if (counter)
		mType = STRUCTERED_COUNTER;
	mHeapType = heaptype;
	mBufferSize = buffersize;
	GpuAddress = mResource->GetGPUVirtualAddress();

	if (mHeapType == D3D12_HEAP_TYPE_DEFAULT)  // if the buffer is in defaul area, need another heap for upload
	{
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(buffersize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mUploadBuffer));
		if (FAILED(hr))
			return false;
	}
	



	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = elementcount;
	srvDesc.Buffer.StructureByteStride = strideSize;
	if (raw)
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	mSRV = heap.addResource(SRV, mResource, &srvDesc, nullptr);
	if (type == STRUCTERED_BUFFER_TYPE_READ_WRITE)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		if (counter)
			uavDesc.Buffer.CounterOffsetInBytes = buffersize - sizeof(UINT);
		else
			uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = elementcount;
		uavDesc.Buffer.StructureByteStride = strideSize;
		if (raw)
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		if (counter)
			mUAV = heap.addResource(UAV, mResource, &uavDesc, mResource);
		else
			mUAV = heap.addResource(UAV, mResource, &uavDesc, nullptr);
	}

	return true;
}



void Buffer::maptoCpu()
{

	if (mHeapType != D3D12_HEAP_TYPE_UPLOAD)
	{
		cout << "Only resource in Upload Heap can map to CPU" << endl;
	}
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(mResource->Map(0, &readRange, &CpuMapAddress));


}
void Buffer::unMaptoCpu()
{
	CD3DX12_RANGE readRange(0, 0);
	CpuMapAddress = nullptr;
	mResource->Unmap(0,&readRange);
}

void Buffer::updateBufferfromCpu(void const* data, UINT size)
{
	if (CpuMapAddress)
	{
		memcpy(CpuMapAddress, data, size);
	}
	else
		cout << "Not mapping to CPU, need to map first" << endl;
}