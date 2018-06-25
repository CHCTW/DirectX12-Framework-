#include "Render.h"
#include "Window.h"
#include "stdafx.h"
#include <string>
#include <algorithm>
#include "../GraphicsUtility/CubeCamera.h"

//WCHAR* downSampleShader = "";
#define MipMapChainBLockSize 16u



static Pipeline DownSamplePipeline;
Render::Render():mDevice(NULL), mSwapChainAccout(0),
mDxgiFactory(NULL), mDxgiAdaptor(NULL), mSwapChain(NULL), mSwapChainRenderTarget(NULL)
{
	mCommandQueue[COMMAND_TYPE_GRAPHICS] = nullptr;
	mCommandQueue[COMMAND_TYPE_COMPUTE] = nullptr;
	mCommandQueue[COMMAND_TYPE_COPY] = nullptr;
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
	hr = mDevice->CreateCommandQueue(&queueDes, IID_PPV_ARGS(&mCommandQueue[COMMAND_TYPE_GRAPHICS]));
	queueDes.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	hr = mDevice->CreateCommandQueue(&queueDes, IID_PPV_ARGS(&mCommandQueue[COMMAND_TYPE_COMPUTE]));
	queueDes.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	hr = mDevice->CreateCommandQueue(&queueDes, IID_PPV_ARGS(&mCommandQueue[COMMAND_TYPE_COPY]));


	
	if (!SUCCEEDED(hr))
	{
		std::cout << "Fail to create command queue" << std::endl;
		return false;
	}

	//NAME_D3D12_OBJECT(mCommandQueue);
	mFence.initialize(*this);
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
	
	mMipmapsig.mParameters.resize(2);
	mMipmapsig.mParameters[0].mType = PARAMETERTYPE_ROOTCONSTANT; // mips level
	mMipmapsig.mParameters[0].mBindSlot = 0;
	mMipmapsig.mParameters[0].mResCounts = 2;
	mMipmapsig.mParameters[1].mType = PARAMETERTYPE_UAV; // input+ouput, both uav state
	mMipmapsig.mParameters[1].mBindSlot = 0;
	mMipmapsig.mParameters[1].mResCounts = 2;
	mMipmapsig.initialize(this->mDevice);

	std::string shaderpath("Shaders/DirectX12-Framework/");

	ShaderSet downsampleshaders[MIP_MAP_GEN_COUNT];
	for (int i = 0; i < MIP_MAP_GEN_COUNT; ++i)
	{
		downsampleshaders[i].shaders[CS].load((shaderpath + MipMapShadersName[i]).c_str(), "CSMain", CS);
		mMipmapPipelines[i].createComputePipeline(this->mDevice, mMipmapsig, downsampleshaders[i]);
	}


	mCubeMipmapsig.mParameters.resize(5);
	mCubeMipmapsig.mParameters[0].mType = PARAMETERTYPE_CBV; // mips level
	mCubeMipmapsig.mParameters[0].mBindSlot = 0;
	mCubeMipmapsig.mParameters[0].mResCounts = 1;
	mCubeMipmapsig.mParameters[1].mType = PARAMETERTYPE_ROOTCONSTANT; // mips level
	mCubeMipmapsig.mParameters[1].mBindSlot = 1;
	mCubeMipmapsig.mParameters[1].mResCounts = 1;
	mCubeMipmapsig.mParameters[2].mType = PARAMETERTYPE_SRV; // input+ouput, both uav state
	mCubeMipmapsig.mParameters[2].mBindSlot = 0;
	mCubeMipmapsig.mParameters[2].mResCounts = 1;
	mCubeMipmapsig.mParameters[3].mType = PARAMETERTYPE_UAV; // input+ouput, both uav state
	mCubeMipmapsig.mParameters[3].mBindSlot = 0;
	mCubeMipmapsig.mParameters[3].mResCounts = 1;
	mCubeMipmapsig.mParameters[4].mType = PARAMETERTYPE_SAMPLER; // input+ouput, both uav state
	mCubeMipmapsig.mParameters[4].mBindSlot = 0;
	mCubeMipmapsig.mParameters[4].mResCounts = 1;

	mCubeMipmapsig.initialize(this->mDevice);


	ShaderSet cubedownsampleshaders[CUBE_MIP_MAP_GEN_COUNT];
	for (int i = 0; i < CUBE_MIP_MAP_GEN_COUNT; ++i)
	{
		cubedownsampleshaders[i].shaders[CS].load((shaderpath + CubeMipMapShadersName[i]).c_str(), "CSMain", CS);
		mCubeMipmapPipelines[i].createComputePipeline(this->mDevice, mCubeMipmapsig, cubedownsampleshaders[i]);
	}
	return true;
}



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


	HRESULT hr = mDxgiFactory->CreateSwapChain(mCommandQueue[COMMAND_TYPE_GRAPHICS], &swapChainDesc, &tempSwapChain);
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
		mMipmapPipelines[i].release();
	}
	mMipmapsig.realease();


	mCubeMipmapsig.realease();
	for (int i = 0; i < CUBE_MIP_MAP_GEN_COUNT; ++i)
	{
		mCubeMipmapPipelines[i].release();
	}
	CloseHandle(mFenceEvent);
	mFence.release();
	mDSVDescriptorHeap.release();
	mRTVDescriptorHeap.release();
	for (int i = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
		mDescriptorHeaps[i].release();
	SAFE_RELEASE(mCommandQueue[COMMAND_TYPE_GRAPHICS]);
	SAFE_RELEASE(mCommandQueue[COMMAND_TYPE_COMPUTE]);
	SAFE_RELEASE(mCommandQueue[COMMAND_TYPE_COPY]);
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
	//cmds[0].mType

	
	mCommandQueue[cmds[0].mType]->ExecuteCommandLists(counts, lists);
	//mCommandQueue[COMMAND_TYPE_GRAPHICS]->ExecuteCommandLists(counts, lists);
}


void Render::insertSignalFence(Fence& fence, CommandType cmdtype)
{
	mCommandQueue[cmdtype]->Signal(fence.mDx12Fence, fence.fenceValue);
	fence.state = FENCE_STATE_INSERTED;
}
void Render::waitFenceIncreament(Fence& fence)
{
//	HANDLE handle;
	if (fence.state == FENCE_STATE_INSERTED)
	{
		if (fence.mDx12Fence->GetCompletedValue() < fence.fenceValue)
		{
			fence.mDx12Fence->SetEventOnCompletion(fence.fenceValue, fence.event);
			WaitForSingleObject(fence.event, INFINITE);
		}
		++fence.fenceValue;
		fence.state = FENCE_STATE_FINISHED;
	}
}
void Render::waitFence(Fence& fence)
{
	if (fence.state == FENCE_STATE_INSERTED)
	{
		if (fence.mDx12Fence->GetCompletedValue() < fence.fenceValue)
		{
			fence.mDx12Fence->SetEventOnCompletion(fence.fenceValue, fence.event);
			WaitForSingleObject(fence.event, INFINITE);
		}
		fence.state = FENCE_STATE_FINISHED;
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

void Render::generateMipMapOffline(Texture& texture, Mip_Map_Generate_Type type, UINT genstartlevel, UINT genendlevel)
{

	
	unsigned int levelto = min(genendlevel, (UINT)texture.textureDesc.MipLevels-1);
	DescriptorHeap tempHeaps;
	tempHeaps.ininitialize(this->mDevice, 1); // a temp descriptor heap
	CommandAllocator cmdalloc;
	CommandList cmdlist;
	cmdalloc.initialize(this->mDevice);
	cmdlist.initial(this->mDevice, cmdalloc);
	Fence tempfenses;
	tempfenses.initialize(*this);
	Texture gentexture;
	vector<D3D12_RESOURCE_STATES> srcprevstate = texture.mState;

	if (texture.mSRVType == TEXTURE_SRV_TYPE_2D)
	{
		struct SRCSlice
		{
			unsigned int slicenum;
			unsigned int miplevel;
		} srcslice;
		gentexture.CreateTexture(*this, tempHeaps, texture.mFormat, texture.textureDesc.Width, texture.textureDesc.Height, texture.textureDesc.DepthOrArraySize, texture.textureDesc.MipLevels, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV, TEXTURE_ALL_MIPS_USE_UAV);
		cmdalloc.reset();
		cmdlist.reset(mMipmapPipelines[type]);
		// copy data;
		cmdlist.bindDescriptorHeaps(&tempHeaps);
		cmdlist.resourceTransition(texture, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cmdlist.resourceTransition(gentexture, D3D12_RESOURCE_STATE_COPY_DEST,true);
		cmdlist.copyResource(texture, gentexture);
		cmdlist.resourceTransition(gentexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		cmdlist.bindComputeRootSigature(mMipmapsig, false);
		for (srcslice.miplevel = genstartlevel; srcslice.miplevel <= levelto; ++srcslice.miplevel)
		{
		
			cmdlist.setBarrier();
			//const Resource && t = gentexture;
			cmdlist.bindComputeResource(1, gentexture, srcslice.miplevel-1);// bind miplevel-1 and miplevel, miplevel-1 is src texture, miplevel is desttexture
			unsigned int width = gentexture.mLayouts[srcslice.miplevel].Footprint.Width; // since each slice should have the same format, can always take the first slice
			unsigned int height = gentexture.mLayouts[srcslice.miplevel].Footprint.Height;
			for (srcslice.slicenum = 0; srcslice.slicenum < texture.textureDesc.DepthOrArraySize; ++srcslice.slicenum)
			{
				cmdlist.bindComputeConstant(0, &srcslice);
				cmdlist.dispatch((width + MipMapChainBLockSize - 1) / MipMapChainBLockSize, (height + MipMapChainBLockSize - 1) / MipMapChainBLockSize, 1);

			}
			cmdlist.UAVWait(gentexture, true); // wait for current mip level finish writing
		}
		cmdlist.resourceTransition(gentexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cmdlist.resourceTransition(texture, D3D12_RESOURCE_STATE_COPY_DEST, true);
		cmdlist.copyResource(gentexture,texture);
		for (int i = 0; i < srcprevstate.size(); ++i)
		{
			cmdlist.resourceTransition(texture, srcprevstate[i],false,i);
		}
		cmdlist.setBarrier();


		cmdlist.close();
		this->executeCommands(&cmdlist);
	//	UINT64 getcurrent = mFence.mDx12Fence->GetCompletedValue();
		this->insertSignalFence(tempfenses);
		this->waitFenceIncreament(tempfenses);
		
	}
	else 
	{
		std::cout << "Wrong Texture Type to generate Mip Map" << std::endl;
	}

	gentexture.release();
	tempHeaps.release();
	cmdalloc.release();
	cmdlist.release();
	tempfenses.release();
	

}
void Render::generateCubeMipMapOffline(Texture& texture, Cube_Mip_Map_Generate_Type type, UINT genstartlevel, UINT genendlevel )
{
	unsigned int levelto = min(genendlevel, (UINT)texture.textureDesc.MipLevels - 1);
	DescriptorHeap tempHeaps;
	DescriptorHeap tempSamplerHeaps;
	tempHeaps.ininitialize(this->mDevice, 1); // a temp descriptor heap
	tempSamplerHeaps.ininitialize(this->mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	DynamicConstantBuffer constantbuffer;
	constantbuffer.initialize(*this, sizeof(CubeViewProjection));
	constantbuffer.setCurrentFrameNumber(1);
	CubeCamera cubecamera;
	VolatileConstantBuffer volconsbuf = constantbuffer.allocateVolatileConstantBuffer(cubecamera.getMatrix(), sizeof(CubeViewProjection));

	Sampler sampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);
	sampler.createSampler(tempSamplerHeaps);

	CommandAllocator cmdalloc;
	CommandList cmdlist;
	cmdalloc.initialize(this->mDevice);
	cmdlist.initial(this->mDevice, cmdalloc);
	Fence tempfenses;
	tempfenses.initialize(*this);
	Texture gentexture;
	vector<D3D12_RESOURCE_STATES> srcprevstate = texture.mState;
	if (texture.mSRVType == TEXTURE_SRV_TYPE_CUBE)
	{
		cout << "Generate for texture Cube" << endl;
		struct SRCSlice
		{
			unsigned int slicenum;
			unsigned int miplevel;
		} srcslice;
		CubeCamera capture;
		gentexture.CreateTexture(*this, tempHeaps, texture.mFormat, texture.textureDesc.Width, texture.textureDesc.Height, texture.textureDesc.DepthOrArraySize, texture.textureDesc.MipLevels, TEXTURE_SRV_TYPE_CUBE, TEXTURE_USAGE_SRV_UAV, TEXTURE_ALL_MIPS_USE_UAV);
		cmdalloc.reset();
		cmdlist.reset(mCubeMipmapPipelines[type]);
		// copy data;
		cmdlist.bindDescriptorHeaps(&tempHeaps,&tempSamplerHeaps);
		cmdlist.resourceTransition(texture, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cmdlist.resourceTransition(gentexture, D3D12_RESOURCE_STATE_COPY_DEST, true);
		cmdlist.copyResource(texture, gentexture);
		cmdlist.resourceTransition(gentexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		cmdlist.bindComputeRootSigature(mCubeMipmapsig, false);
		cmdlist.bindComputeResource(0, volconsbuf);
		cmdlist.bindComputeSampler(4, sampler);





		cmdlist.close();
		this->executeCommands(&cmdlist);
		//	UINT64 getcurrent = mFence.mDx12Fence->GetCompletedValue();
		this->insertSignalFence(tempfenses);
		this->waitFenceIncreament(tempfenses);
		//cmdlist
	}

	constantbuffer.release();
	tempSamplerHeaps.release();
	gentexture.release();
	tempHeaps.release();
	cmdalloc.release();
	cmdlist.release();
	tempfenses.release();
}
void Render::updateBufferOffline(Buffer& destbuffer, void const * data, UINT64 datasize, UINT64 bufferoffset)
{
	CommandAllocator cmdalloc;
	CommandList cmdlist;
	cmdalloc.initialize(this->mDevice);
	cmdlist.initial(this->mDevice, cmdalloc);
	Fence tempfenses;
	tempfenses.initialize(*this);
	vector<D3D12_RESOURCE_STATES> bufferstate = destbuffer.mState;
	DynamicUploadBuffer uploadbuffer;
	uploadbuffer.initialize(*this, datasize);

	cmdalloc.reset();
	cmdlist.reset();
	cmdlist.resourceTransition(destbuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
	cmdlist.updateBufferData(uploadbuffer, destbuffer, data, datasize, bufferoffset);

	for (int i = 0; i < bufferstate.size(); ++i)
	{
		cmdlist.resourceTransition(destbuffer, bufferstate[i],false,i);
	}
	cmdlist.setBarrier();
	cmdlist.close();

	
	this->executeCommands(&cmdlist);
	//	UINT64 getcurrent = mFence.mDx12Fence->GetCompletedValue();
	this->insertSignalFence(tempfenses);
	this->waitFenceIncreament(tempfenses);


	uploadbuffer.release();
	cmdalloc.release();
	cmdlist.release();
	tempfenses.release();

}
void Render::updateTextureOffline(Texture& texture, void  const * data, UINT startlevel, UINT levelnum, UINT startslice, UINT slicenum)
{
	CommandAllocator cmdalloc;
	CommandList cmdlist;
	cmdalloc.initialize(this->mDevice);
	cmdlist.initial(this->mDevice, cmdalloc);
	Fence tempfenses;
	tempfenses.initialize(*this);
	vector<D3D12_RESOURCE_STATES> 	texturestate = texture.mState;
	DynamicUploadBuffer uploadbuffer;
	uploadbuffer.initialize(*this);


	cmdalloc.reset();
	cmdlist.reset();
	cmdlist.resourceTransition(texture, D3D12_RESOURCE_STATE_COPY_DEST, true);
	cmdlist.updateTextureData(uploadbuffer, texture, data, startlevel, levelnum, startslice, slicenum);

	for (int i = 0; i < texturestate.size(); ++i)
	{
		cmdlist.resourceTransition(texture, texturestate[i], false, i);
	}

	cmdlist.setBarrier();
	cmdlist.close();


	this->executeCommands(&cmdlist);
	//	UINT64 getcurrent = mFence.mDx12Fence->GetCompletedValue();
	this->insertSignalFence(tempfenses);
	this->waitFenceIncreament(tempfenses);


	uploadbuffer.release();
	cmdalloc.release();
	cmdlist.release();
	tempfenses.release();
}