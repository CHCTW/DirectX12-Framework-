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


#include <stb\stb_image.h>
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

float lod = 0;

void initializeRender()
{


	//Camera cam;



	render.initialize();
	RenderTargetFormat retformat(false);
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
	rootsig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootsig.mParameters[3].mConstantData = &lod;


	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/CubeMapMipMapChain/SkyBoxMips.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/CubeMapMipMapChain/SkyBoxMips.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.setRatio(1.0f);
	camera.setFOV(90);
	camera.updateViewProj();
}

void loadAsset()
{
	sampler.createSampler(samplerheap);


	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();

	Image img[6];
	img[0].load("Assets/Textures/right.jpg", 10);
	img[1].load("Assets/Textures/left.jpg", 10);
	img[2].load("Assets/Textures/top.jpg", 10);
	img[3].load("Assets/Textures/bottom.jpg", 10);
	img[4].load("Assets/Textures/back.jpg", 10);
	img[5].load("Assets/Textures/front.jpg", 10);



	int width, height, bpp;
	//	unsigned char* rgb = stbi_load("Assets/Textures/front.jpg", &width, &height, &bpp, 4);
	unsigned char* skyboxdata[6];

	skyboxdata[0] = img[0].mData;
	skyboxdata[1] = img[1].mData;
	skyboxdata[2] = img[2].mData;
	skyboxdata[3] = img[3].mData;
	skyboxdata[4] = img[4].mData;
	skyboxdata[5] = img[5].mData;

	//skyBox.CreateTexture(render.mDevice, DXGI_FORMAT_R8G8B8A8_UNORM, img[0].mWidth, img[0].mHeight, 1, true,10);



	//skyBox.addSahderResorceView(srvheap);


	skyBox.CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, img[0].mWidth, img[0].mHeight, 1, 10, TEXTURE_SRV_TYPE_CUBE);


	import.ReadFile("Assets/sphere.obj", aiProcessPreset_TargetRealtime_Fast);
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
	cmdlist.reset();
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

	cmdlist.updateTextureData(skyBox, skyboxdata[0], 0, 1, 0, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[1], 0, 1, 1, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[2], 0, 1, 2, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[3], 0, 1, 3, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[4], 0, 1, 4, 1);
	cmdlist.updateTextureData(skyBox, skyboxdata[5], 0, 1, 5, 1);


	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	cmdlist.resourceTransition(skyBox, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);
	//	import.FreeScene();

}

void releaseRender()
{
	depthBuffer[0].release();
	depthBuffer[1].release();
	depthBuffer[2].release();
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
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex], depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	//cmdlist.clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//cmdlist.bindIndexBuffer(indexBuffer);
	//cmdlist.bindVertexBuffers(vertexBuffer, normalBuffer);
	cmdlist.drawInstance(3, 1, 0, 0);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, true);
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
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		press = true;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
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
		lod += 0.1;
	if (key == GLFW_KEY_LEFT)
		lod -= 0.1;
	if (lod > 10)
		lod = 10;
	if (lod < 0)
		lod = 0;

}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "Cube Mipmap Chain");
	windows.openWindow();
	glfwSetCursorPosCallback(windows.mWindow, cursor_pos_callback);
	glfwSetMouseButtonCallback(windows.mWindow, mouse_button_callback);
	glfwSetScrollCallback(windows.mWindow, scroll_callback);
	glfwSetKeyCallback(windows.mWindow, key_callback);

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