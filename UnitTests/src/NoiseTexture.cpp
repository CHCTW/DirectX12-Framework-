#include <iostream>
#include <vector>
#include <algorithm>
#include "Window.h"
#include "Render.h"
#include "StructureHeaders.h"
#include "GPUNoiseTextureGenerator.h"
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
Buffer indexBuffer;
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
DescriptorHeap srvheap;
DescriptorHeap samplerheap;
Texture texture;
static uint32_t swapChainCount = 3;
const UINT TextureWidth = 256;
const UINT TextureHeight = 256;

ShaderSet shaderset;
Sampler sampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
GPUNoiseTextureGenerator noiseGenerator;
const UINT noiseTextureWidth = 256;
const UINT noiseTextureHeight = 256;
Texture NoiseTexture2D;
float animatetime = 0;

void initializeRender()
{
	render.initialize();
	RenderTargetFormat rtformat;
	noiseGenerator.initialize(render);
	render.createSwapChain(windows, swapChainCount, rtformat.mRenderTargetFormat[0]);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render);

	fence.fenceValue = 1;

	fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);

	srvheap.ininitialize(render.mDevice, 1);
	samplerheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	if (!vertexBuffer.createVertexBuffer(render.mDevice, 4 * 4 * sizeof(float), 4 * sizeof(float)))
	{
		cout << "Fail for create vertex buffer" << endl;
	}
	if (!indexBuffer.createIndexBuffer(render.mDevice, sizeof(UINT) * 6))
	{
		cout << "Fail for create index buffer" << endl;
	}
	NoiseTexture2D.CreateTexture(render, srvheap, DXGI_FORMAT_R32_FLOAT, noiseTextureWidth, noiseTextureHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);
	


	texture.CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, TextureWidth, TextureHeight);
	//texture.addSahderResorceView(srvheap);
	rootsig.mParameters.resize(2);
	rootsig.mParameters[0].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &NoiseTexture2D;
	rootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootsig.mParameters[1].mType = PARAMETERTYPE_SAMPLER;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 0;
	rootsig.mParameters[1].mSampler = &sampler;


	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/NoiseTexture/NoiseTexture.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/NoiseTexture/NoiseTexture.hlsl", "PSMain", PS);
	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, rtformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState());

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

}

void loadAsset()
{

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
		-0.25f,-0.25f,0.0f,1.0f,
		0.0f,-0.75f,0.5f,0.0f

	};
	unsigned int indexdata[] = { 0,1,2,2,1,3 };


	sampler.createSampler(samplerheap);

	cmdalloc.reset();
	cmdlist.reset();
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(texture, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
	cmdlist.updateBufferData(vertexBuffer, tridata, 4 * 4 * sizeof(float));
	cmdlist.updateBufferData(indexBuffer, indexdata, 6 * sizeof(int));
	cmdlist.updateTextureData(texture, &(data[0]));
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, true);
	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);
}

void releaseRender()
{

	indexBuffer.release();
	samplerheap.release();
	texture.release();
	srvheap.release();
	pipeline.release();
	rootsig.realease();
	NoiseTexture2D.release();
	vertexBuffer.release();
	fence.release();
	cmdlist.release();
	cmdalloc.release();
	render.releaseSwapChain();
	render.release();
	noiseGenerator.release();
	CloseHandle(fenceEvet);
}

void update()
{
	animatetime += 0.01f;
	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset();

	cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);

	cmdlist.resourceTransition(NoiseTexture2D, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
	noiseGenerator.recordPureGenerateWorleyNoise2D(cmdlist, NoiseTexture2D,false,5, animatetime);
	cmdlist.resourceTransition(NoiseTexture2D, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

	cmdlist.bindPipeline(pipeline);
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
}
int main()
{
	windows.initialize(1600, 900, "Noise Texture");
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