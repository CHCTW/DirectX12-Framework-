#pragma once
#include "d3dx12.h"
class CommandAllocator
{
public:
	CommandAllocator();
	bool initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE= D3D12_COMMAND_LIST_TYPE_DIRECT);
	void release();
	bool reset();
	ID3D12CommandAllocator* mDx12Allocater;
	D3D12_COMMAND_LIST_TYPE mType;
};