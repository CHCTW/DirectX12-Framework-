#pragma once
#include "d3dx12.h"
#include "stdafx.h"
#include "Resource.h"
class DescriptorHeap;
class Render;
class Buffer :public Resource
{
public:
	Buffer();
	BufferType mType;
	D3D12_HEAP_TYPE mHeapType;
	UINT mBufferSize;
	bool mCounter;
	//bool createBuffer(ID3D12Device* device, UINT buffersize, D3D12_HEAP_TYPE heaptype = D3D12_HEAP_TYPE_DEFAULT);
	bool createVertexBuffer(ID3D12Device* device,UINT buffersize, UINT strideSize,D3D12_HEAP_TYPE heaptype = D3D12_HEAP_TYPE_DEFAULT);
	bool createIndexBuffer(ID3D12Device* device, UINT buffersize, IndexBufferFormat indexformat = INDEX_BUFFER_FORMAT_32BIT, D3D12_HEAP_TYPE heaptype = D3D12_HEAP_TYPE_DEFAULT);
	//make stride size 32bytes make more efficieny
	bool createStructeredBuffer(ID3D12Device* device, DescriptorHeap &heap, UINT strideSize, UINT elementcount, StructeredBufferType type, bool counter = false, bool raw = false, bool padding = false, D3D12_HEAP_TYPE heaptype = D3D12_HEAP_TYPE_DEFAULT);
	bool createConstantBuffer(ID3D12Device* device, DescriptorHeap &heap,UINT buffersize);
	bool createConstantBuffer(ID3D12Device* device, UINT buffersize);
	// a temp function to move constant buffer to defaul heap function, soon or later, there will be no upload buffer in any resource, dynamic upload buffer and offline upload functions will replace it
	bool createConstanBufferNew(Render &render, DescriptorHeap &heap, UINT buffersize);

	void maptoCpu();
	void unMaptoCpu();
	void updateBufferfromCpu(void const* data,UINT size);
	union
	{
		VertexBufferView mVertexBuffer;
		IndexBufferView mIndexBuffer;
	};
};