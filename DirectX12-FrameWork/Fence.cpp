#include "Fence.h"
#include "stdafx.h"
Fence::Fence():mDx12Fence(NULL)
{

}
bool Fence::initialize(ID3D12Device* device, D3D12_FENCE_FLAGS flag)
{
	HRESULT hr;

	hr = device->CreateFence(0, flag, IID_PPV_ARGS(&mDx12Fence));
	if (FAILED(hr))
	{
		cout << "Fail to create Fence" << endl;
		return false;
	}
	Flag = flag;
	fenceValue = 0;
	return true;
}
void Fence::release()
{
	SAFE_RELEASE(mDx12Fence);
}