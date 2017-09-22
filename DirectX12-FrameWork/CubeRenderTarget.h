#pragma once
#include "stdafx.h"
#include "d3dx12.h"
#include "Texture.h"
#include <vector>
using namespace std;
class DescriptorHeap;
enum CubeRenderTargetType
{
	CUBE_RENDERTAERGET_TYPE_DEPTH = 0x01,
	CUBE_RENDERTAERGET_TYPE_RENDERTARGET = 0x02
};

class CubeRenderTarget
{

public:

	bool createCubeRenderTargets(ID3D12Device* device, UINT width, UINT height, CubeRenderTargetType type, DescriptorHeap& heap, DescriptorHeap& srvuavheap, D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_NONE, DXGI_FORMAT renderforamt = DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT depthforamt = DXGI_FORMAT_R32_TYPELESS);
	bool createCubeRenderTargets(ID3D12Device* device, UINT width, UINT height, CubeRenderTargetType type, DescriptorHeap& rtvheap, DescriptorHeap& dsvheap, DescriptorHeap& srvuavheap, D3D12_RESOURCE_FLAGS renderflag = D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_FLAGS depthflag = D3D12_RESOURCE_FLAG_NONE, DXGI_FORMAT renderforamt = DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT depthforamt = DXGI_FORMAT_R32_TYPELESS);
	//	bool mDepth;
//	bool createRenderTargets(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& rtvheap, DescriptorHeap& dsvheap, DescriptorHeap& cbvsrvuavheap);
//	bool createRenderTargets(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& heap, DescriptorHeap& cbvsrvuavheap);
	//	bool createCubeDepthBuffers(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& heap, DescriptorHeap& cbvsrvuavheap);
	//	bool createRenderTargets(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& dsvheap, DescriptorHeap& cbvsrvuavheap);
	CubeRenderTargetType mType;
	UINT mWidth;
	UINT mHeight;
	DXGI_FORMAT mRenderTargetFormat;
	vector<Texture> mRenderBuffer;
	Handles mFaceRTV[6];
	ClearValue mRenderTargetClearValue;
	vector<Texture> mDepthBuffer;
	DXGI_FORMAT mDepthFormat;
	Handles mFaceDSV[6];
	ClearValue mDepthStencilClearValue;
	void release();

};
