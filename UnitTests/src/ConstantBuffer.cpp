#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <chrono>
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
Buffer constantBuffer;
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
DescriptorHeap cbvsrvheap;
DescriptorHeap samplerheap;
Texture texture;
static uint32_t swapChainCount = 3;
const UINT TextureWidth = 256;
const UINT TextureHeight = 256;

ShaderSet shaderset;
Sampler sampler;
float offset = 0;

long  long current;
long  long before;
void initializeRender()
{
	render.initialize();
	RenderTargetFormat rtformat;
	render.createSwapChain(windows, swapChainCount, rtformat.mRenderTargetFormat[0]);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render);

	cbvsrvheap.ininitialize(render.mDevice, 1);

	samplerheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	if (!vertexBuffer.createVertexBuffer(render.mDevice, 3 * 4 * sizeof(float), 4 * sizeof(float)))
	{
		cout << "Fail for create" << endl;
	}

	constantBuffer.createConstantBuffer(render.mDevice, cbvsrvheap,sizeof(float));
	


//	texture.CreateTexture(render.mDevice, DXGI_FORMAT_R8G8B8A8_UNORM, TextureWidth, TextureHeight);
	texture.CreateTexture(render, cbvsrvheap, DXGI_FORMAT_R8G8B8A8_UNORM, TextureWidth, TextureHeight);

//	texture.addSahderResorceView(cbvsrvheap);
	rootsig.mParameters.resize(3);
	rootsig.mParameters[0].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &texture;
	rootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootsig.mParameters[1].mType = PARAMETERTYPE_SAMPLER;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 0;
	rootsig.mParameters[1].mSampler = &sampler;
	rootsig.mParameters[2].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[2].mResCounts = 1;
	rootsig.mParameters[2].mBindSlot = 0;
	rootsig.mParameters[2].mResource = &constantBuffer;
	rootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/ConstantBuffer.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/ConstantBuffer.hlsl", "PSMain", PS);
	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, rtformat,DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState());

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

}

void loadAsset()
{


	constantBuffer.maptoCpu();
//	constantBuffer.updateBufferfromCpu(&offset, sizeof(offset));
	const UINT rowPitch = TextureWidth * 4;
	const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
	const UINT cellHeight = TextureWidth >> 3;	// The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * TextureHeight;
	
	std::vector<UINT8> data(textureSize);
	UINT8* pData = &data[0];

	for (UINT n = 0; n < textureSize; n += 4)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;		// R
			pData[n + 1] = 0x00;	// G
			pData[n + 2] = 0x00;	// B
			pData[n + 3] = 0xff;	// A
		}
		else
		{
			pData[n] = 0xff;		// R
			pData[n + 1] = 0xff;	// G
			pData[n + 2] = 0xff;	// B
			pData[n + 3] = 0xff;	// A
		}
	}


	float tridata[] =
	{
		0.0f,0.25f,0.5f,0.0f,
		0.25f,-0.25f,1.0f,1.0f,
		-0.25f,-0.25f,0.0f,1.0f
	};
	sampler.createSampler(samplerheap);

	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(texture, D3D12_RESOURCE_STATE_COPY_DEST, true);
	cmdlist.updateBufferData(vertexBuffer, tridata, 3 * 4 * sizeof(float));
	cmdlist.updateTextureData(texture, &(data[0]));
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);
}

void releaseRender()
{


	constantBuffer.release();
	samplerheap.release();
	texture.release();
	cbvsrvheap.release();
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
//	cout<<std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()<<endl;
//	current = std::chrono::duration_cast<std::chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch()).count();
//	current = GetTickCount();
//	cout << current - before << endl;
//	windows.setTitle(to_string(current-before));
	//system("pause");

	offset += 0.001f;
	if (offset >= 1.5)
		offset = -1.5;
	constantBuffer.updateBufferfromCpu(&offset, sizeof(offset));
	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(pipeline);
	cmdlist.bindDescriptorHeaps(&cbvsrvheap, &samplerheap);
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex]);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist.bindVertexBuffer(vertexBuffer);
	cmdlist.drawInstance(3, 1, 0, 0);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, true);
	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.present();
	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);
//	before = current;
}
int main()
{
	int testnumber = 3;
	windows.initialize(1280, 720, "ConstantBuffer");
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