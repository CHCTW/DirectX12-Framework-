#pragma once
#include "stdafx.h"
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <vector>
using namespace std;
struct SwapChainBuffer
{
	DXGI_MODE_DESC backbufferdesc;
	vector<D3D12_RESOURCE_STATES> mState;
	ID3D12Resource* mResource;
	vector<Handles> mRTV;
};