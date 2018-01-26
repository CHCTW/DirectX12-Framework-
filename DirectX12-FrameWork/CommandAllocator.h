#pragma once
#include "d3dx12.h"
#include "stdafx.h"
class CommandAllocator
{
public:
	CommandAllocator();
	bool initialize(ID3D12Device* device, CommandType cmdtype= COMMAND_TYPE_GRAPHICS);
	void release();
	bool reset();
	ID3D12CommandAllocator* mDx12Allocater;
	CommandType mType;
};