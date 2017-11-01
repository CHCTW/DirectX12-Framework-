#pragma once
#include "stdafx.h"
#include "d3dx12.h"
#include <vector>
class RootSignature;

// all command should keep, draw, drawindex, dispatch at the last
class CommandSignature
{
public:
	CommandSignature() : mCommandSignature(nullptr), paddingsize(0)
	{

	}
	bool initialize(ID3D12Device* device, RootSignature& rootsig);
	void release();
	std::vector<CommandParameter> mParameters;
	D3D12_COMMAND_SIGNATURE_DESC mCommandSignatureDes;
	ID3D12CommandSignature* mCommandSignature;
	UINT paddingsize;
};