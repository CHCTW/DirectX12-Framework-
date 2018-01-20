#pragma once
#include "d3dx12.h"
#include "stdafx.h"
#include "Resource.h"
#include <iostream>

using namespace std;
class DescriptorHeap;
class Render;
enum TEXTURE_USAGE
{
	TEXTURE_USAGE_SRV = 0x01,
	TEXTURE_USAGE_UAV = 0x02,
	TEXTURE_USAGE_RTV = 0x04,
	TEXTURE_USAGE_DSV = 0x08,
	TEXTURE_USAGE_SRV_DSV = TEXTURE_USAGE_SRV| TEXTURE_USAGE_DSV,
	TEXTURE_USAGE_SRV_UAV = TEXTURE_USAGE_SRV | TEXTURE_USAGE_UAV,
	TEXTURE_USAGE_SRV_RTV = TEXTURE_USAGE_SRV | TEXTURE_USAGE_RTV,
};
enum TEXTURE_SRV_TYPE
{
	TEXTURE_SRV_TYPE_2D,  // if depth > 1 , it's a 2dtexturearray
	TEXTURE_SRV_TYPE_CUBE, // if depth > 1 , it's an array of cube that is total num is 6*depth 
	TEXTURE_SRV_TYPE_3D
};

enum TEXTURE_ALL_MIPS_USE
{
	TEXTURE_ALL_MIPS_USE_NONE = 0x00,
	TEXTURE_ALL_MIPS_USE_UAV = 0x01,
	TEXTURE_ALL_MIPS_USE_RTV = 0x02,
	TEXTURE_ALL_MIPS_USE_DSV = 0x04,
	TEXTURE_ALL_MIPS_USE_ALL = TEXTURE_ALL_MIPS_USE_UAV| TEXTURE_ALL_MIPS_USE_RTV| TEXTURE_ALL_MIPS_USE_DSV
};


class Texture : public Resource
{
public:
	Texture();
	/*void CreateTexture(ID3D12Device* device, DXGI_FORMAT format,UINT width, UINT height, UINT arraySize = 1,bool isCubeMap = false, UINT mipLevel = 1, D3D12_RESOURCE_FLAGS usage = D3D12_RESOURCE_FLAG_NONE,ClearValue& clear = DefaultClearValue,D3D12_RESOURCE_DIMENSION dem = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
	 D3D12_RESOURCE_STATES state= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);*/

	void CreateTexture(Render& render, DescriptorHeap& srvcbvheap,DXGI_FORMAT format, UINT width, UINT height, UINT arraySize = 1,UINT mipLevel = 1,
		TEXTURE_SRV_TYPE srvtype = TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE usage = TEXTURE_USAGE_SRV,TEXTURE_ALL_MIPS_USE mipuse = TEXTURE_ALL_MIPS_USE_NONE,
		ClearValue& clear = DefaultClearValue, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	void release();
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
	TEXTURE_USAGE mTextUsage;
	TEXTURE_ALL_MIPS_USE mMipsUse;
	TEXTURE_SRV_TYPE mSRVType;
	DXGI_FORMAT mFormat;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* mLayouts;
	//Handles mSRV;
	//Handles mUAV;
private:
	void createSRV(DescriptorHeap& heap);
	void createRTVs(DescriptorHeap& heap);
	void createDSVs(DescriptorHeap& heap);
	void createUAVs(DescriptorHeap& heap);
};