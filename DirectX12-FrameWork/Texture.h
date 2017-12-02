#pragma once
#include "d3dx12.h"
#include "stdafx.h"
#include "Resource.h"
#include <iostream>
using namespace std;
class DescriptorHeap;


class Texture : public Resource
{
public:
	Texture();
	void CreateTexture(ID3D12Device* device, DXGI_FORMAT format,UINT width, UINT height, UINT arraySize = 1,bool isCubeMap = false, UINT mipLevel = 1, D3D12_RESOURCE_FLAGS usage = D3D12_RESOURCE_FLAG_NONE,ClearValue& clear = DefaultClearValue,D3D12_RESOURCE_DIMENSION dem = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
	 D3D12_RESOURCE_STATES state= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	
	void addSahderResorceView(DescriptorHeap& heap);
	void addDepgthStencilView(DescriptorHeap& heap,UINT level  = 0);
	void addRenderTargetView(DescriptorHeap& heap);
	void addUnorderedAccessView(DescriptorHeap& heap);
	UINT mByteSize;
	bool mCubeMap;
	//D3D12_RESOURCE_STATES mState;
	//ID3D12Resource* mResource;
	//ID3D12Resource* mUploadBuffer;
	D3D12_RESOURCE_DESC textureDesc;
	D3D12_CLEAR_VALUE mClearVal;
	D3D12_RESOURCE_FLAGS mUsage;
	DXGI_FORMAT mFormat;
	//Handles mSRV;
	//Handles mUAV;

};