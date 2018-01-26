#include "CommandAllocator.h"
#include "stdafx.h"
CommandAllocator::CommandAllocator():mDx12Allocater(NULL)
{

}
bool CommandAllocator::initialize(ID3D12Device* device,CommandType cmdtype)
{
	HRESULT hr;
	mType = cmdtype;
	switch (cmdtype)
	{
	case COMMAND_TYPE_GRAPHICS:
			hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mDx12Allocater));
			break;
	case COMMAND_TYPE_COMPUTE:
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&mDx12Allocater));
		break;
	case COMMAND_TYPE_COPY:
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&mDx12Allocater));
		break;
	}
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