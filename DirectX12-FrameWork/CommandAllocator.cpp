#include "CommandAllocator.h"
#include "stdafx.h"
CommandAllocator::CommandAllocator():mDx12Allocater(NULL), mType(D3D12_COMMAND_LIST_TYPE_DIRECT)
{

}
bool CommandAllocator::initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
{
	HRESULT hr;
	hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&mDx12Allocater));
	if (FAILED(hr))
	{
		cout << "Fail to create Command Allocator" << endl;
		return false;
	}
	return true;
}
bool CommandAllocator::reset()
{
	HRESULT hr;
	hr = mDx12Allocater->Reset();
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}
void CommandAllocator::release()
{
	SAFE_RELEASE(mDx12Allocater);
}