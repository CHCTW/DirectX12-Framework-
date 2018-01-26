#include "Fence.h"
#include "stdafx.h"
#include "StructureHeaders.h"
Fence::Fence():mDx12Fence(NULL)
{

}
bool Fence::initialize(Render& render, D3D12_FENCE_FLAGS flag)
{
	HRESULT hr;

	hr = render.mDevice->CreateFence(0, flag, IID_PPV_ARGS(&mDx12Fence));
	if (FAILED(hr))
	{
		cout << "Fail to create Fence" << endl;
		return false;
	}
	event = CreateEvent(NULL, FALSE, FALSE, NULL);
	Flag = flag;
	fenceValue = 1;
	insert = false;
	return true;
}
void Fence::release()
{
	SAFE_RELEASE(mDx12Fence);
}