#include"QueryHeap.h"
bool QueryHeap::initialize(ID3D12Device* device, D3D12_QUERY_HEAP_TYPE type, UINT count)
{
	mQueryDes.Count = count;
	mQueryDes.Type = type;
	mQueryDes.NodeMask = 0;
	ThrowIfFailed(device->CreateQueryHeap(&mQueryDes, IID_PPV_ARGS(&mHeap)));
	return true;
}
void QueryHeap::release()
{
	SAFE_RELEASE(mHeap);
}