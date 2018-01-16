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
	// when genlevels = -1, means generate to the end of the level in texture, this should only use for offline
	void generateMipMapOffline(Texture& texture, Mip_Map_Generate_Type type, UINT start = 0,UINT gentolevel = -1);
	bool present();
	
	UINT mSwapChainAccout;
	ID3D12Device* mDevice;
	
	// going to combine these three together
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


	
private:
	//Structure for mip-map generate block
	RootSignature mMipmapsig;
	Pipeline mMipmapPipelines[MIP_MAP_GEN_COUNT];
};
