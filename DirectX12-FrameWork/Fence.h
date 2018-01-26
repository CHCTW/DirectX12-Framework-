#pragma once
#include "d3dx12.h"
class Render;
class Fence
{
public:
	Fence();
	bool initialize(Render& render, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE);
	void release();
	ID3D12Fence* mDx12Fence;
	UINT64 fenceValue;
	D3D12_FENCE_FLAGS Flag;
	HANDLE event;
	bool insert;
};