//#pragma once
//#include "stdafx.h"
//#include "d3dx12.h"
//#include "Texture.h"
//#include <vector>
//using namespace std;
//class DescriptorHeap;
//
//
//class RenderTarget
//{
//public:
////	bool mDepth;
//	bool createRenderTargets(ID3D12Device* device, UINT width, UINT height,RenderTargetFormat& format, DescriptorHeap& rtvheap, DescriptorHeap& dsvheap, DescriptorHeap& cbvsrvuavheap);
//	bool createRenderTargets(ID3D12Device* device, UINT width, UINT height,RenderTargetFormat& format, DescriptorHeap& heap, DescriptorHeap& cbvsrvuavheap);
////	bool createCubeDepthBuffers(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& heap, DescriptorHeap& cbvsrvuavheap);
////	bool createRenderTargets(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& dsvheap, DescriptorHeap& cbvsrvuavheap);
//	UINT mWidth;
//	UINT mHeight;
//	RenderTargetFormat mFormat;
//	vector<Texture> mRenderBuffers;
//	vector<Texture> mDepthBuffer;
//	void release();
//
//};