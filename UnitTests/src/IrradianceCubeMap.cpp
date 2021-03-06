#include <iostream>
#include <vector>
#include <algorithm>
#include <assimp\ai_assert.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\mesh.h>
#include "Window.h"
#include "Render.h"
#include "StructureHeaders.h"
#include "SpecCamera.h"
#include "Image.h"
#include <stb/stb_image.h>
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
Buffer normalBuffer;
Buffer indexBuffer;
Buffer cameraBuffer;
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
DescriptorHeap srvheap;
DescriptorHeap rtvheap;
static uint32_t swapChainCount = 3;
vector<Texture> depthBuffer;
ShaderSet shaderset;
Assimp::Importer import;
SpecCamera camera;
aiScene const * scene = nullptr;
aiMesh* mesh = nullptr;
float curxpos;
float curypos;
float curxoffet;
float curyoffet;
bool press = false;


Texture skyBox;
Sampler sampler;
DescriptorHeap samplerheap;

Texture IrradianceMap;
const UINT mapWidth = 256;
const UINT mapHeight = 256;
bool irr = true;
unsigned face = 0;

void initializeRender()
{


	//Camera cam;



	render.initialize();
	RenderTargetFormat retformat(true);
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render);


	srvheap.ininitialize(render.mDevice, 1);

	depthBuffer.resize(swapChainCount);
	depthBuffer[0].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[1].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[2].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);


	samplerheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);



	rtvheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	rootsig.mParameters.resize(4);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;
	rootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 0;
	rootsig.mParameters[1].mResource = &skyBox;
	rootsig.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootsig.mParameters[2].mType = PARAMETERTYPE_SAMPLER;
	rootsig.mParameters[2].mResCounts = 1;
	rootsig.mParameters[2].mBindSlot = 0;
	rootsig.mParameters[2].mSampler = &sampler;
	rootsig.mParameters[3].mType = PARAMETERTYPE_ROOTCONSTANT;
	rootsig.mParameters[3].mResCounts = 1;
	rootsig.mParameters[3].mBindSlot = 1;
	rootsig.mParameters[3].mConstantData = &face;

	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/SkyBox.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/SkyBox.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);
	camera.setFOV(90);
	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.updateViewProj();
}

void loadAsset()
{
	sampler.createSampler(samplerheap);


	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();

//	IrradianceMap.createCubeRenderTargets(render.mDevice, mapWidth, mapHeight,1,1, CUBE_RENDERTAERGET_TYPE_RENDERTARGET, rtvheap, srvheap, D3D12_RESOURCE_FLAG_NONE, DXGI_FORMAT_R32G32B32A32_FLOAT);
	IrradianceMap.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, mapWidth, mapHeight, 1, 1, TEXTURE_SRV_TYPE_CUBE, TEXTURE_USAGE_SRV_RTV);

	int width, height, bpp;
	//	unsigned char* rgb = stbi_load("Assets/Textures/front.jpg", &width, &height, &bpp, 4);
	float* skyboxdata[6];

	//skyboxdata[0] = stbi_loadf("Assets/Textures/milk0003.hdr", &width, &height, &bpp, 4);
	//skyboxdata[1] = stbi_loadf("Assets/Textures/milk0001.hdr", &width, &height, &bpp, 4);
	//skyboxdata[2] = stbi_loadf("Assets/Textures/milk0005.hdr", &width, &height, &bpp, 4);
	//skyboxdata[3] = stbi_loadf("Assets/Textures/milk0004.hdr", &width, &height, &bpp, 4);
	//skyboxdata[4] = stbi_loadf("Assets/Textures/milk0006.hdr", &width, &height, &bpp, 4);
	//skyboxdata[5] = stbi_loadf("Assets/Textures/milk0002.hdr", &width, &height, &bpp, 4);

	Image img[6];
	img[0].load("Assets/Textures/milk0003.hdr", 5);
	img[1].load("Assets/Textures/milk0001.hdr", 5);
	img[2].load("Assets/Textures/milk0005.hdr", 5);
	img[3].load("Assets/Textures/milk0004.hdr", 5);
	img[4].load("Assets/Textures/milk0006.hdr", 5);
	img[5].load("Assets/Textures/milk0002.hdr", 5);





	////	unsigned char* rgb = stbi_load("Assets/Textures/front.jpg", &width, &height, &bpp, 4);

	skyboxdata[0] = (float *)img[0].mData;
	skyboxdata[1] = (float *)img[1].mData;
	skyboxdata[2] = (float *)img[2].mData;
	skyboxdata[3] = (float *)img[3].mData;
	skyboxdata[4] = (float *)img[4].mData;
	skyboxdata[5] = (float *)img[5].mData;

	//img[0].load("Assets/Textures/right.jpg", 10);
	//img[1].load("Assets/Textures/left.jpg", 10);
	//img[2].load("Assets/Textures/top.jpg", 10);
	//img[3].load("Assets/Textures/bottom.jpg", 10);
	//img[4].load("Assets/Textures/back.jpg", 10);
	//img[5].load("Assets/Textures/front.jpg", 10);


//	skyBox.CreateTexture(render.mDevice, DXGI_FORMAT_R8G8B8A8_UNORM, img[0].mWidth, img[0].mHeight, 6, true);
	//skyBox.CreateTexture(render.mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, img[0].mWidth, img[0].mHeight, 6, true,5);

	//skyBox.addSahderResorceView(srvheap);

	skyBox.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, img[0].mWidth, img[0].mHeight, 1, 5, TEXTURE_SRV_TYPE_CUBE);




	import.ReadFile("Assets/cube.obj", aiProcessPreset_TargetRealtime_Fast);
	scene = import.GetScene();
	mesh = scene->mMeshes[0];
	mesh->HasPositions();
	UINT verticnum = mesh->mNumVertices;

	if (!vertexBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float)))
	{
		cout << "Fail for create" << endl;
	}
	normalBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));


	indexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * 3 * mesh->mNumFaces);

	std::vector<unsigned int> indexdata;
	indexdata.resize(mesh->mNumFaces * 3);
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		indexdata[i * 3] = mesh->mFaces[i].mIndices[0];
		indexdata[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		indexdata[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}



	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(skyBox, D3D12_RESOURCE_STATE_COPY_DEST, true);

	cmdlist.updateBufferData(vertexBuffer, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(normalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	//cmdlist.updateTextureCubeData(skyBox, (void const **)skyboxdata);

	cmdlist.updateTextureData(skyBox, skyboxdata[0], 0, 5, 0, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[1], 0, 5, 1, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[2], 0, 5, 2, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[3], 0, 5, 3, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[4], 0, 5, 4, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[5], 0, 5, 5, 1);


	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(skyBox, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(IrradianceMap, D3D12_RESOURCE_STATE_GENERIC_READ, true);


	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);

}
void generateIrrMap()
{

//	RootSignature irradiancerootsig;

//	irradiancerootsig.mParameters.resize(3);


	ShaderSet irrshaders;
	irrshaders.shaders[VS].load("Shaders/IrradianceCubeMap.hlsl", "VSMain", VS);
	irrshaders.shaders[PS].load("Shaders/IrradianceCubeMap.hlsl", "PSMain", PS);
	irrshaders.shaders[GS].load("Shaders/IrradianceCubeMap.hlsl", "GSMain", GS);

	Pipeline IrraidancePipeline;
	RenderTargetFormat retformat(DXGI_FORMAT_R32G32B32A32_FLOAT);

	IrraidancePipeline.createGraphicsPipeline(render.mDevice, rootsig, irrshaders, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);
	
	ViewPort tempviewport;
	Scissor tempscissor;
	tempviewport.setup(0.0f, 0.0f, (float)mapWidth, (float)mapHeight);
	tempscissor.setup(0, mapWidth, 0, mapHeight);


	
	Camera views[6];
	views[0].setTarget(-1.0, 0.0, 0.0);
	views[0].setUp(0.0, -1.0, 0.0);

	views[1].setTarget(1.0, 0.0, 0.0);
	views[1].setUp(0.0, -1.0, 0.0);

	views[2].setTarget(0.0, -1.0, 0.0);
	views[2].setUp(0.0, 0.0, 1.0);

	views[3].setTarget(0.0, 1.0, 0.0);
	views[3].setUp(0.0, 0.0, -1.0);

	views[4].setTarget(0.0, 0.0, -1.0);
	views[4].setUp(0.0, -1.0, 0.0);
	views[5].setTarget(0.0, 0.0, 1.0);
	views[5].setUp(0.0, -1.0, 0.0);


	for (int i = 0; i < 6; ++i)
	{
		views[i].setFOV(90);
		views[i].setRatio(1.0);
		views[i].updateViewProj();

	}





	for (; face < 6; ++face)
	{

		cmdalloc.reset();
		cmdlist.reset(IrraidancePipeline);

		cmdlist.resourceTransition(IrradianceMap,D3D12_RESOURCE_STATE_RENDER_TARGET,true);

		cmdlist.setViewPort(tempviewport);
		cmdlist.setScissor(tempscissor);
		cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);
		cmdlist.bindGraphicsRootSigature(rootsig);
		cameraBuffer.updateBufferfromCpu(views[face].getMatrix(), sizeof(ViewProjection));
		//cmdlist.bindCubeRenderTarget(IrradianceMap, face);
		cmdlist.bindRenderTargetsOnly(IrradianceMap);
		cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdlist.bindIndexBuffer(indexBuffer);
		cmdlist.bindVertexBuffers(vertexBuffer, normalBuffer);
		cmdlist.drawIndexedInstanced(mesh->mNumFaces * 3, 1, 0, 0);

	//	cmdlist.cubeRenderTargetBarrier(IrradianceMap, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

		cmdlist.resourceTransition(IrradianceMap, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		cmdlist.close();
		render.executeCommands(&cmdlist);
		render.insertSignalFence(fence);
		render.waitFenceIncreament(fence);

	
	}
	IrraidancePipeline.release();



}
void releaseRender()
{
	depthBuffer[2].release();
	depthBuffer[1].release();
	depthBuffer[0].release();
	IrradianceMap.release();
	rtvheap.release();
	samplerheap.release();
	skyBox.release();
	import.FreeScene();
	cameraBuffer.release();
	indexBuffer.release();
	normalBuffer.release();
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
	camera.updateViewProj();
	cameraBuffer.updateBufferfromCpu(camera.getMatrix(), sizeof(ViewProjection));
	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(pipeline);
	cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	if(irr)
		cmdlist.bindGraphicsResource(1, IrradianceMap);
	else 
		cmdlist.bindGraphicsResource(1, skyBox);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex],depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist.bindIndexBuffer(indexBuffer);
	cmdlist.bindVertexBuffers(vertexBuffer, normalBuffer);
	cmdlist.drawIndexedInstanced(mesh->mNumFaces * 3, 1, 0, 0);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT,true);
	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.present();


	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);
}


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	curxoffet = xpos - curxpos;
	curyoffet = ypos - curypos;

	curxpos = xpos;
	curypos = ypos;

	if (press)
		camera.addAngle(curxoffet / 5.0, curyoffet / 5.0);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT&& action == GLFW_PRESS)
		press = true;
	if (button == GLFW_MOUSE_BUTTON_RIGHT&& action == GLFW_RELEASE)
		press = false;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset == 1)
		camera.addZoom(1);
	if (yoffset == -1)
		camera.addZoom(-1);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_RIGHT)
		irr = true;
	if (key == GLFW_KEY_LEFT)
		irr = false;


}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "Irradiance Map");
	windows.openWindow();
	glfwSetCursorPosCallback(windows.mWindow, cursor_pos_callback);
	glfwSetMouseButtonCallback(windows.mWindow, mouse_button_callback);
	glfwSetScrollCallback(windows.mWindow, scroll_callback);
		glfwSetKeyCallback(windows.mWindow, key_callback);

	int limit = 10000;
	int count = 0;
	initializeRender();
	loadAsset();
	generateIrrMap();
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