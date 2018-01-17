#include "Render.h"
#include "Window.h"
#include "stdafx.h"
#include <string>
#include <algorithm>
//WCHAR* downSampleShader = "";




static Pipeline DownSamplePipeline;
Render::Render():mDevice(NULL), mCommandQueue(NULL), mSwapChainAccout(0),
mDxgiFactory(NULL), mDxgiAdaptor(NULL), mSwapChain(NULL), mSwapChainRenderTarget(NULL)
{

}
bool Render::initialize()
{

	int adapterIndex = 0;
	bool adapterFound = false;
	uint32_t dxgiFactoryFlags = 0;
	HRESULT hr;
#if defined( _DEBUG )
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugLayer))))
	{
		mDebugLayer->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	

	hr = CreateDXGIFactory2(dxgiFactoryFlags,IID_PPV_ARGS(&mDxgiFactory));
	if (FAILED(hr))
	{
		std::cout << "Create Fail";
	}


	while (mDxgiFactory->EnumAdapters1(adapterIndex, &mDxgiAdaptor) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		mDxgiAdaptor->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			adapterIndex++; 
			continue;
		}
		hr = D3D12CreateDevice(mDxgiAdaptor, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			hr =  D3D12CreateDevice(mDxgiAdaptor, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
			break;
		}
		mDxgiAdaptor->Release();
		adapterIndex++;
	}
	


	//mDevice->CheckFeatureSupport
	//std::cout << "Fail to Create Device" << std::endl;
	//crearte command queue 
	D3D12_COMMAND_QUEUE_DESC queueDes = {};
	queueDes.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDes.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDes.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDes.NodeMask = 0;
	hr = mDevice->CreateCommandQueue(&queueDes, IID_PPV_ARGS(&mCommandQueue));
	
	if (!SUCCEEDED(hr))
	{
		std::cout << "Fail to create command queue" << std::endl;
		return false;
	}

	//NAME_D3D12_OBJECT(mCommandQueue);
	mFence.initialize(mDevice);
	mFence.fenceValue = 1;
	mFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	for (int i = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
	{
		if (!mDescriptorHeaps[i].ininitialize(mDevice, HeapSizes[i], (D3D12_DESCRIPTOR_HEAP_TYPE)i))
			return false;
	}
	if (!mRTVDescriptorHeap.ininitialize(mDevice, 10, D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
		return false;

	if (!mDSVDescriptorHeap.ininitialize(mDevice, 10, D3D12_DESCRIPTOR_HEAP_TYPE_DSV))
		return false;



	// create pipelien for generate  mipmpaps
	
	mMipmapsig.mParameters.resize(3);
	mMipmapsig.mParameters[0].mType = PARAMETERTYPE_ROOTCONSTANT; // mips level
	mMipmapsig.mParameters[0].mBindSlot = 0;
	mMipmapsig.mParameters[0].mResCounts = 1;
	mMipmapsig.mParameters[1].mType = PARAMETERTYPE_SRV; // input
	mMipmapsig.mParameters[1].mBindSlot = 0;
	mMipmapsig.mParameters[1].mResCounts = 1;
	mMipmapsig.mParameters[2].mType = PARAMETERTYPE_UAV; // ouput
	mMipmapsig.mParameters[2].mBindSlot = 0;
	mMipmapsig.mParameters[2].mResCounts = 1;
	mMipmapsig.initialize(this->mDevice);

	std::string shaderpath("Shaders/DirectX12-Framework/");

	ShaderSet downsampleshaders[MIP_MAP_GEN_COUNT];
	for (int i = 0; i < MIP_MAP_GEN_COUNT; ++i)
	{
		downsampleshaders[i].shaders[CS].load((shaderpath + MipMapShadersName[i]).c_str(), "CSMain", CS);
		mMipmapPipelines[i].createComputePipeline(this->mDevice, mMipmapsig, downsampleshaders[i]);
	}
	return true;
}
//bool Render::createSwapChain(Window &window, UINT  count, RenderTargetFormat &format)
//{
//	DXGI_MODE_DESC backbufferdesc = {};
//	backbufferdesc.Format = format.mRenderTargetFormat[0];
//	backbufferdesc.Height = window.mHeight;
//	backbufferdesc.Width = window.mWidth;
//	
//	DXGI_SAMPLE_DESC samdesc = {};
//	samdesc.Count = 1;
//
//	IDXGISwapChain* tempSwapChain;
//
//	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
//	swapChainDesc.BufferCount = count;
//	swapChainDesc.BufferDesc = backbufferdesc;
//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
//	swapChainDesc.OutputWindow = glfwGetWin32Window(window.mWindow);
//	swapChainDesc.SampleDesc = samdesc;
//	swapChainDesc.Windowed = true;
//
//
//	//DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
//	//swapChainDesc.BufferCount = count;
//	//swapChainDesc.Width = window.mWidth;
//	//swapChainDesc.Height = window.mHeight;
//	//swapChainDesc.Format = format.mRenderTargetFormat[0];
//	//swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//	//swapChainDesc.SampleDesc.Count = 1;
//	
//
//	HRESULT hr = mDxgiFactory->CreateSwapChain(mCommandQueue, &swapChainDesc, &tempSwapChain);
////	HRESULT hr = mDxgiFactory->CreateSwapChainForHwnd(mCommandQueue, glfwGetWin32Window(window.mWindow), &swapChainDesc, nullptr, nullptr, &tempSwapChain);
//	if (!SUCCEEDED(hr))
//	{
//		std::cout << "Fail to Create Swap Chain" << std::endl;
//		return false;
//	}
//	mSwapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
//	
//	mSwapChainRenderTarget = new RenderTarget[3];
//
//	for (UINT i = 0; i < count; ++i)
//	{
//		mSwapChainRenderTarget[i].mWidth = window.mWidth;
//		mSwapChainRenderTarget[i].mHeight = window.mHeight;
//		ClearValue depthclear;
//		depthclear.DepthStencil.Depth = 1.0f;
//
//
//
//		mSwapChainRenderTarget[i].mFormat = format;
//		mSwapChainRenderTarget[i].mRenderBuffers.resize(1);
//		hr = mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainRenderTarget[i].mRenderBuffers[0].mResource));
//		mSwapChainRenderTarget[i].mRenderBuffers[0].mState.push_back(D3D12_RESOURCE_STATE_RENDER_TARGET);
//		mSwapChainRenderTarget[i].mRenderBuffers[0].mFormat = format.mRenderTargetFormat[0];
//		mSwapChainRenderTarget[i].mRenderBuffers[0].mRTV.push_back (mRTVDescriptorHeap.addResource(RTV, mSwapChainRenderTarget[i].mRenderBuffers[0].mResource, NULL));
//		if (format.mDepth) 
//		{
//			mSwapChainRenderTarget[i].mDepthBuffer.resize(1);
//			mSwapChainRenderTarget[i].mDepthBuffer[0].CreateTexture(mDevice, format.mDepthStencilFormat, window.mWidth, window.mHeight, 1, false, 1,D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, depthclear, D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_DEPTH_WRITE);
//			mSwapChainRenderTarget[i].mDepthBuffer[0].addDepgthStencilView(mDSVDescriptorHeap);
//		}
//		if (FAILED(hr))
//		{
//			std::cout << "Fail to get Buffer from SwapChain" << std::endl;
//			return false;
//		}
//	}
//	mSwapChainAccout = count;  //  setup swap chain account
//	return true;
//}


bool Render::createSwapChain(Window& window, UINT  count, DXGI_FORMAT format)
{
	DXGI_MODE_DESC backbufferdesc = {};
	backbufferdesc.Format = format;
	backbufferdesc.Height = window.mHeight;
	backbufferdesc.Width = window.mWidth;

	DXGI_SAMPLE_DESC samdesc = {};
	samdesc.Count = 1;

	IDXGISwapChain* tempSwapChain;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = count;
	swapChainDesc.BufferDesc = backbufferdesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.OutputWindow = glfwGetWin32Window(window.mWindow);
	swapChainDesc.SampleDesc = samdesc;
	swapChainDesc.Windowed = true;


	HRESULT hr = mDxgiFactory->CreateSwapChain(mCommandQueue, &swapChainDesc, &tempSwapChain);
	//	HRESULT hr = mDxgiFactory->CreateSwapChainForHwnd(mCommandQueue, glfwGetWin32Window(window.mWindow), &swapChainDesc, nullptr, nullptr, &tempSwapChain);
	if (!SUCCEEDED(hr))
	{
		std::cout << "Fail to Create Swap Chain" << std::endl;
		return false;
	}
	mSwapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

	mSwapChainRenderTarget.resize(count);
	D3D12_RENDER_TARGET_VIEW_DESC rtvdesc;
	rtvdesc.Format = format;
	rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvdesc.Texture2D.MipSlice = 0;
	rtvdesc.Texture2D.PlaneSlice = 0;

	for (UINT i = 0; i < count; ++i)
	{
		/*mSwapChainRenderTarget[i].mWidth = window.mWidth;
		mSwapChainRenderTarget[i].mHeight = window.mHeight;
		ClearValue depthclear;
		depthclear.DepthStencil.Depth = 1.0f;*/
		hr = mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainRenderTarget[i].mResource));
		mSwapChainRenderTarget[i].backbufferdesc = backbufferdesc;
		mSwapChainRenderTarget[i].mState.push_back(D3D12_RESOURCE_STATE_PRESENT);
		mSwapChainRenderTarget[i].mRTV.push_back(mRTVDescriptorHeap.addResource(RTV, mSwapChainRenderTarget[i].mResource, NULL));

		if (FAILED(hr))
		{
			std::cout << "Fail to get Buffer from SwapChain" << std::endl;
			return false;
		}
	}
	mSwapChainAccout = count;  //  setup swap chain account
	return true;
}
UINT Render::getCurrentSwapChainIndex()
{
	return mSwapChain->GetCurrentBackBufferIndex();
}


void Render::release()
{

	for (int i = 0; i < MIP_MAP_GEN_COUNT; ++i)
	{
		mMipmapPipelines->release();
	}
	mMipmapsig.realease();

	CloseHandle(mFenceEvent);
	mFence.release();
	mDSVDescriptorHeap.release();
	mRTVDescriptorHeap.release();
	for (int i = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
		mDescriptorHeaps[i].release();
	SAFE_RELEASE(mCommandQueue);
	SAFE_RELEASE(mDevice);
	SAFE_RELEASE(mDxgiFactory);
	SAFE_RELEASE(mDxgiAdaptor);
#if defined( _DEBUG )
	SAFE_RELEASE(mDebugLayer);
#endif
}

void Render::releaseSwapChain()
{
	for (UINT i = 0; i < mSwapChainAccout; ++i)
		SAFE_RELEASE(mSwapChainRenderTarget[i].mResource);
	SAFE_RELEASE(mSwapChain);
}

void Render::executeCommands(CommandList *cmds, UINT counts)
{
	//mCommandQueue->exe
	ID3D12CommandList* lists[MaxSubmitCommandList];
	for (UINT i = 0; i < counts; ++i)
	{
		lists[i] = cmds[i].mDx12CommandList;
		cmds[i].mCurrentBindGraphicsRootSig = nullptr;
		cmds[i].mCurrentBindComputeRootSig = nullptr;
	}
	mCommandQueue->ExecuteCommandLists(counts, lists);
}
void Render::waitCommandsDone()
{
	const UINT64 fenval = mFence.fenceValue;
	mCommandQueue->Signal(mFence.mDx12Fence, fenval);
	mFence.fenceValue++;

	if (mFence.mDx12Fence->GetCompletedValue() < fenval)
	{
		mFence.mDx12Fence->SetEventOnCompletion(fenval, mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
	}
}
bool Render::present()
{
	HRESULT hr;
	hr = mSwapChain->Present(0, 0);
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

void Render::generateMipMapOffline(Texture& texture, Mip_Map_Generate_Type type, UINT baselevel, UINT gentolevel)
{

	
	unsigned int levelto = min(gentolevel, texture.textureDesc.MipLevels-1);
	DescriptorHeap tempHeaps;
	tempHeaps.ininitialize(this->mDevice, 1); // a temp descriptor heap
	CommandAllocator cmdalloc;
	CommandList cmdlist;
	cmdalloc.initialize(this->mDevice);
	cmdlist.initial(this->mDevice, cmdalloc);


//	Texture gentexture;


	//vector<Handles> uavs;
	//D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	//D3D12_RESOURCE_BARRIER barrier;
	//barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//barrier.Transition.pResource = texture.mResource;
	//barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	//barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	//vector<D3D12_RESOURCE_BARRIER> barriers;
	//uavDesc.Format = texture.textureDesc.Format;
	//if (uavDesc.Format == DXGI_FORMAT_R32_TYPELESS)
	//	uavDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//if (uavDesc.Format == DXGI_FORMAT_R16_TYPELESS)
	//	uavDesc.Format = DXGI_FORMAT_R16_UNORM;


	//if (texture.textureDesc.DepthOrArraySize > 1)
	//{
	//	cout << "Only Simple Texture 2D is supported now" << endl;
	//}
	//else
	//{
	//	// allocate temporary uav for each level
	//	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	//	uavDesc.Texture2D.PlaneSlice = 0;
	//	for (int j = baselevel+1; j <= levelto; ++j)
	//	{
	//		uavDesc.Texture2D.MipSlice = j;
	//		uavs.push_back(tempHeaps.addResource(UAV, texture.mResource, &uavDesc, nullptr));
	//	}
	//}

	////cmdlist.mDx12CommandList->CopyResource


	tempHeaps.release();
	cmdalloc.release();
	cmdlist.release();

	

}