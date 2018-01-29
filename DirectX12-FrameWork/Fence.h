#pragma once
#include "d3dx12.h"
class Render;
enum FenceState
{
	FENCE_STATE_UNUSED,
	FENCE_STATE_INSERTED,
	FENCE_STATE_FINISHED
};
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
	FenceState state;
	void updateValue(UINT64 value)
	{
		fenceValue = value;
	}
	UINT64 getValue()
	{
		return fenceValue;
	}
	bool finished()
	{
		return state== FENCE_STATE_FINISHED;
	}

};