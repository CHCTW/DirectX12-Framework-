#pragma once
#define D3DCOMPILE_DEBUG 1
#include <dxgi1_4.h>
#include <d3d12.h>
#include <iostream>
#include <wrl.h>
#include <dxgidebug.h>
#include <d3d12sdklayers.h>
#include "StructureHeaders.h"
class Window;
using Microsoft::WRL::ComPtr;
class Render
{
public:
	
	Render();
	bool initialize();
	void release();
	// swap chain particualr for present render target on the window
	bool createSwapChain(Window& window, UINT  count,RenderTargetFormat & format);
	void releaseSwapChain();
	UINT getCurrentSwapChainIndex();
	void executeCommands(CommandList *cmds, UINT counts = 1);
	void waitCommandsDone();
	bool present();
	
	UINT mSwapChainAccout;
	ID3D12Device* mDevice;
	
	ID3D12CommandQueue* mCommandQueue;
	Fence mFence;
	HANDLE mFenceEvent;
	IDXGIFactory4* mDxgiFactory;
	IDXGIAdapter1* mDxgiAdaptor;
	IDXGISwapChain3* mSwapChain;
	RenderTarget* mSwapChainRenderTarget;
	DescriptorHeap mRTVDescriptorHeap;
	DescriptorHeap mDSVDescriptorHeap;
	//UINT mRTVDescSize;
	
	DescriptorHeap mDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	
	ID3D12Debug* mDebugLayer;

};
