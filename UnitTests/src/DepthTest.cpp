#include <iostream>
#include <vector>
#include <algorithm>
#include "Window.h"
#include "Render.h"
#include "StructureHeaders.h"
using namespace std;
Render render;
CommandAllocator cmdalloc;
CommandList cmdlist;
Window windows;
Pipeline pipeline;
Fence fence;
HANDLE fenceEvet;
UINT frameIndex;
Buffer vertexBuffer;
Buffer constBuffer;
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
vector<Texture> depthBuffer;
DescriptorHeap srvheap;

static uint32_t swapChainCount = 3;
const UINT TextureWidth = 256;
const UINT TextureHeight = 256;

ShaderSet shaderset;
Sampler sampler;
float offset = 0.0f;
void initializeRender()
{
	render.initialize();
	RenderTargetFormat rtformat(true);
	render.createSwapChain(windows, swapChainCount, rtformat.mRenderTargetFormat[0]);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render.mDevice);

	fence.fenceValue = 1;

	fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);

	//dsvheap.ininitialize(render.mDevice,1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	srvheap.ininitialize(render.mDevice, 1);


	depthBuffer.resize(swapChainCount);
	depthBuffer[0].CreateTexture(render, srvheap, rtformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[1].CreateTexture(render, srvheap, rtformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[2].CreateTexture(render, srvheap, rtformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);


	if (!vertexBuffer.createVertexBuffer(render.mDevice, 6 * 6 * sizeof(float), 6 * sizeof(float)))
	{
		cout << "Fail for create" << endl;
	}
	constBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(float));
	constBuffer.maptoCpu();


	rootsig.mParameters.resize(1);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &constBuffer;


	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/DepthTest.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/DepthTest.hlsl", "PSMain", PS);
	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, rtformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState());

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

}

void loadAsset()
{




	float tridata[] =
	{
		0.0f,0.25f,0.5f,1.0f,0.0f,0.0f,
		0.25f,-0.25f,0.5f,1.0f,0.0f,0.0f,
		-0.25f,-0.25f,0.5f,1.0f,0.0f,0.0f,
		0.3f,0.25f,0.9f,0.0f,1.0f,0.0f,
		0.55f,-0.25f,0.9f,0.0f,1.0f,0.0f,
		0.05f,-0.25f,0.9f,0.0f,1.0f,0.0f
	};


	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.updateBufferData(vertexBuffer, tridata, 6 * 6 * sizeof(float));
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, true);


	
	cmdlist.close();
	render.executeCommands(&cmdlist);
	const UINT64 fenval = fence.fenceValue;
	render.mCommandQueue->Signal(fence.mDx12Fence, fenval);
	fence.fenceValue++;

	if (fence.mDx12Fence->GetCompletedValue() < fenval)
	{
		fence.mDx12Fence->SetEventOnCompletion(fenval, fenceEvet);
		WaitForSingleObject(fenceEvet, INFINITE);
	}
}

void releaseRender()
{
	depthBuffer[0].release();
	depthBuffer[1].release();
	depthBuffer[2].release();
	constBuffer.release();
	srvheap.release();
	pipeline.release();
	rootsig.realease();
	vertexBuffer.release();
	fence.release();
	cmdlist.release();
	cmdalloc.release();
	render.releaseSwapChain();
	render.release();
	CloseHandle(fenceEvet);
}

void update()
{
	offset+= 0.001f;
	if (offset >= 1.0f)
		offset = -1.0f;
	float zero = 0.0f;
	constBuffer.updateBufferfromCpu(&offset, sizeof(float));


	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(pipeline);

	cmdlist.bindDescriptorHeaps(&srvheap);
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
//	cmdlist.mDx12CommandList->OMSetRenderTargets(1, &(render.mSwapChainRenderTarget[frameIndex].mRTV.Cpu), false, &(depthbuffer.mDSV.Cpu));

//	cmdlist.mDx12CommandList->ClearDepthStencilView(depthbuffer.mDSV.Cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0,nullptr);

//	cmdlist.mDx12CommandList->OMSetRenderTargets(render.mSwapChainRenderTarget[frameIndex].mRTV.Cpu, true, dept)

	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex],depthBuffer[frameIndex]);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist.bindVertexBuffer(vertexBuffer);
	cmdlist.drawInstance(6, 1, 0, 0);
	
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT,true);
	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.present();


	const UINT64 fenval = fence.fenceValue;
	render.mCommandQueue->Signal(fence.mDx12Fence, fenval);
	fence.fenceValue++;

	if (fence.mDx12Fence->GetCompletedValue() < fenval)
	{
		fence.mDx12Fence->SetEventOnCompletion(fenval, fenceEvet);
		WaitForSingleObject(fenceEvet, INFINITE);
	}
}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "DepthTest");
	windows.openWindow();
	int limit = 10000;
	int count = 0;
	initializeRender();
	loadAsset();
	while (windows.isRunning())
	{

		windows.pollInput();
		update();
	}
	releaseRender();
	windows.closeWindow();
	windows.termianate();
	return 0;
}