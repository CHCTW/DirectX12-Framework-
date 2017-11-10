#pragma once
#include "d3dx12.h"
#include "stdafx.h"
class QueryHeap
{
public:
	QueryHeap():mHeap(nullptr)
	{
	}
	bool initialize(ID3D12Device* device,D3D12_QUERY_HEAP_TYPE type, UINT count);
	void release();
	ID3D12QueryHeap* mHeap;
	D3D12_QUERY_HEAP_DESC mQueryDes;
};