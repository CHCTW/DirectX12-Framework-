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
#include <chrono>
using namespace std;
Render render;
CommandAllocator cmdalloc;
CommandList cmdlist;

CommandAllocator compcmdalloc;
CommandList compcmdlist;

Window windows;
Pipeline pipeline;
Fence fence;
Fence comfence;
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
struct Particle
{
	glm::vec3 mPos;
	glm::vec3 mVel;
	glm::vec3 mColor;
	float life;
};
Buffer particleBuffers[3];
UINT currentDraw = 0;
UINT particleNum = 1000000;
struct SceneData
{
	float delta;
	UINT totalNum;
} sceneData;
Buffer sceneBuffer;
RootSignature ParticleRootsig;
RootSignature ParticleDrawRootsig;
Pipeline ParticleDraw;
Pipeline ParticleUpdate;
std::chrono::high_resolution_clock::time_point pre;
void initializeRender()
{


	//Camera cam;



	render.initialize();
	RenderTargetFormat retformat(true);
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	compcmdalloc.initialize(render.mDevice,COMMAND_TYPE_COMPUTE);
	compcmdlist.initial(render.mDevice, compcmdalloc);


	fence.initialize(render);
	comfence.initialize(render);
//	fence.fenceValue = 1;
//	fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);

	srvheap.ininitialize(render.mDevice, 1);
	depthBuffer.resize(swapChainCount);
	depthBuffer[0].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[1].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[2].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);

	rootsig.mParameters.resize(1);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;



	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/ModelLoading.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/ModelLoading.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.addZoom(10);
	camera.updateViewProj();

	ParticleRootsig.mParameters.resize(3);
	ParticleRootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	ParticleRootsig.mParameters[0].mResCounts = 1;
	ParticleRootsig.mParameters[0].mBindSlot = 0;
	ParticleRootsig.mParameters[0].mResource = &sceneBuffer;
	ParticleRootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	ParticleRootsig.mParameters[1].mResCounts = 1;
	ParticleRootsig.mParameters[1].mBindSlot = 0;
	ParticleRootsig.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	ParticleRootsig.mParameters[2].mType = PARAMETERTYPE_UAV;
	ParticleRootsig.mParameters[2].mResCounts = 1;
	ParticleRootsig.mParameters[2].mBindSlot = 0;
	ParticleRootsig.initialize(render.mDevice);

	
//	ParticleDrawRootSig.mParameters.resize(2);
//	ParticleDrawRootSig.
	

	ShaderSet ParUpdateShader;
	ParUpdateShader.shaders[CS].load("Shaders/ParticleUpdate.hlsl", "CSMain", CS);
	ParticleUpdate.createComputePipeline(render.mDevice, ParticleRootsig, ParUpdateShader);



	ParticleDrawRootsig.mParameters.resize(2);
	ParticleDrawRootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	ParticleDrawRootsig.mParameters[0].mResCounts = 1;
	ParticleDrawRootsig.mParameters[0].mBindSlot = 0;
	ParticleDrawRootsig.mParameters[0].mResource = &sceneBuffer;
	ParticleDrawRootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	ParticleDrawRootsig.mParameters[1].mResCounts = 1;
	ParticleDrawRootsig.mParameters[1].mBindSlot = 0;
	ParticleDrawRootsig.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	ParticleDrawRootsig.initialize(render.mDevice);




	ShaderSet ParDrawShader;
	ParDrawShader.shaders[VS].load("Shaders/ParticleDraw.hlsl", "VSMain", VS);
	ParDrawShader.shaders[PS].load("Shaders/ParticleDraw.hlsl", "PSMain", PS);

	ParticleDraw.createGraphicsPipeline(render.mDevice, ParticleDrawRootsig, ParDrawShader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(true), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_NONE_SPLIT, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);



}

void loadAsset()
{



	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();







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

	sceneData.totalNum = particleNum;

	sceneBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(SceneData));
	sceneBuffer.maptoCpu();
	sceneBuffer.updateBufferfromCpu(&sceneData, sizeof(SceneData));

	for (int i = 0; i < 3; ++i)
		particleBuffers[i].createStructeredBuffer(render.mDevice, srvheap, sizeof(Particle), particleNum, STRUCTERED_BUFFER_TYPE_READ_WRITE);

	



	ShaderSet ParIniShaderset;
	ParIniShaderset.shaders[CS].load("Shaders/ParticleInitial.hlsl", "CSMain", CS);
	Pipeline InitPar;
	InitPar.createComputePipeline(render.mDevice, ParticleRootsig, ParIniShaderset);


	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.updateBufferData(vertexBuffer, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(normalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER,true);

	cmdlist.bindPipeline(InitPar);
	cmdlist.bindDescriptorHeaps(&srvheap);
	cmdlist.bindComputeRootSigature(ParticleRootsig);
	//cmdlist.bindGraphicsRootSigature(ParticleRootsig);


	cmdlist.bindComputeResource(2, particleBuffers[0]);
	cmdlist.dispatch((particleNum / 512) + 1, 1, 1);
//	cmdlist.resourceBarrier(particleBuffers[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceTransition(particleBuffers[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(particleBuffers[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(particleBuffers[2], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,true);







	cmdlist.close();
	render.executeCommands(&cmdlist);

	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);
	InitPar.release();


	pre = std::chrono::high_resolution_clock::now();
	//	import.FreeScene();
}

void releaseRender()
{
	depthBuffer[2].release();
	depthBuffer[1].release();
	depthBuffer[0].release();
	ParticleUpdate.release();
	ParticleDraw.release();
	ParticleRootsig.realease();
	ParticleDrawRootsig.realease();
	sceneBuffer.release();
	for (int i = 0; i < 3; ++i)
		particleBuffers[i].release();
	import.FreeScene();
	cameraBuffer.release();
	indexBuffer.release();
	normalBuffer.release();
	srvheap.release();
	pipeline.release();
	rootsig.realease();
	vertexBuffer.release();
	fence.release();
	comfence.release();
	compcmdlist.release();
	compcmdalloc.release();

	cmdlist.release();
	cmdalloc.release();
	render.releaseSwapChain();
	render.release();
	CloseHandle(fenceEvet);
}

void update()
{

	std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
	


	std::chrono::duration<float> delta = t - pre;
	pre = t;

//	cout << delta.count() << endl;
	sceneData.delta = delta.count();
	sceneBuffer.updateBufferfromCpu(&sceneData, sizeof(SceneData));

	camera.updateViewProj();
	cameraBuffer.updateBufferfromCpu(camera.getMatrix(), sizeof(ViewProjection));
	frameIndex = render.getCurrentSwapChainIndex();



	// all resource transition
	cmdalloc.reset();
	cmdlist.reset(pipeline);
	cmdlist.resourceTransition(particleBuffers[(currentDraw + 1) % 3], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	cmdlist.close();

	render.executeCommands(&cmdlist);
	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);



	// compute part
	compcmdalloc.reset();
	compcmdlist.reset(ParticleDraw);
	compcmdlist.bindDescriptorHeaps(&srvheap);
	compcmdlist.bindPipeline(ParticleUpdate);
	compcmdlist.bindComputeRootSigature(ParticleRootsig);
	compcmdlist.bindComputeResource(0, sceneBuffer);
	compcmdlist.bindComputeResource(1, particleBuffers[currentDraw]);
	compcmdlist.bindComputeResource(2, particleBuffers[(currentDraw + 1) % 3]);

//	compcmdlist.resourceTransition(particleBuffers[(currentDraw + 1) % 3], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
	compcmdlist.dispatch(((particleNum / 1024) + 1), 1, 1);
//	compcmdlist.resourceTransition(particleBuffers[(currentDraw + 1) % 3], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,true);
	compcmdlist.close();







	//draw part
	cmdalloc.reset();
	cmdlist.reset(pipeline);
	cmdlist.bindDescriptorHeaps(&srvheap);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
//	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
//	cmdlist.resourceTransition(particleBuffers[currentDraw], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//	cmdlist.resourceTransition(particleBuffers[(currentDraw+1)%3],D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);


	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex],depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(depthBuffer[frameIndex]);



	cmdlist.bindGraphicsRootSigature(ParticleDrawRootsig);
	cmdlist.bindPipeline(ParticleDraw);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	cmdlist.bindGraphicsResource(0, cameraBuffer);
	cmdlist.bindGraphicsResource(1, particleBuffers[currentDraw]);
	cmdlist.drawInstance(particleNum,1, 0, 0);

	//cmdlist.bindPipeline(ParticleUpdate);
	//cmdlist.bindComputeRootSigature(ParticleRootsig);
	//cmdlist.bindComputeResource(0, sceneBuffer);
	//cmdlist.bindComputeResource(1, particleBuffers[currentDraw]);
	//cmdlist.bindComputeResource(2, particleBuffers[(currentDraw+1) % 3]);

	//currentDraw = (currentDraw + 1) % 3;
	////cout << currentDraw << endl;
	//cmdlist.dispatch(((particleNum / 1024) + 1), 1, 1);



	//cmdlist.resourceTransition(particleBuffers[(currentDraw + 1) % 3], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT,true);
	cmdlist.close();

	


	render.executeCommands(&compcmdlist);
	
	render.executeCommands(&cmdlist);
	///fffff
	
	render.insertSignalFence(comfence, COMMAND_TYPE_COMPUTE);
	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);
	render.waitFenceIncreament(comfence);

	//resource transition end
	cmdalloc.reset();
	cmdlist.reset(pipeline);
	cmdlist.resourceTransition(particleBuffers[(currentDraw + 1) % 3], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, true);
	cmdlist.close();

	render.executeCommands(&cmdlist);

	render.present();

	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);


	currentDraw = (currentDraw + 1) % 3;

}
void update2()
{

	std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();



	std::chrono::duration<float> delta = t - pre;
	pre = t;

	//	cout << delta.count() << endl;
	sceneData.delta = delta.count();
	sceneBuffer.updateBufferfromCpu(&sceneData, sizeof(SceneData));

	camera.updateViewProj();
	cameraBuffer.updateBufferfromCpu(camera.getMatrix(), sizeof(ViewProjection));
	frameIndex = render.getCurrentSwapChainIndex();


	// compute part
	compcmdalloc.reset();
	compcmdlist.reset(ParticleDraw);
	compcmdlist.bindDescriptorHeaps(&srvheap);
	compcmdlist.bindPipeline(ParticleUpdate);
	compcmdlist.bindComputeRootSigature(ParticleRootsig);
	compcmdlist.bindComputeResource(0, sceneBuffer);
	compcmdlist.bindComputeResource(1, particleBuffers[currentDraw]);
	compcmdlist.bindComputeResource(2, particleBuffers[(currentDraw + 1) % 3]);

	compcmdlist.resourceTransition(particleBuffers[(currentDraw + 1) % 3], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
	compcmdlist.dispatch(((particleNum / 1024) + 1), 1, 1);
	compcmdlist.resourceTransition(particleBuffers[(currentDraw + 1) % 3], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,true);
	compcmdlist.close();







	//draw part
	cmdalloc.reset();
	cmdlist.reset(pipeline);
	cmdlist.bindDescriptorHeaps(&srvheap);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	//	cmdlist.resourceTransition(particleBuffers[currentDraw], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//	cmdlist.resourceTransition(particleBuffers[(currentDraw+1)%3],D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);


	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex], depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(depthBuffer[frameIndex]);



	cmdlist.bindGraphicsRootSigature(ParticleDrawRootsig);
	cmdlist.bindPipeline(ParticleDraw);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	cmdlist.bindGraphicsResource(0, cameraBuffer);
	cmdlist.bindGraphicsResource(1, particleBuffers[currentDraw]);
	cmdlist.drawInstance(particleNum, 1, 0, 0);

	//cmdlist.bindPipeline(ParticleUpdate);
	//cmdlist.bindComputeRootSigature(ParticleRootsig);
	//cmdlist.bindComputeResource(0, sceneBuffer);
	//cmdlist.bindComputeResource(1, particleBuffers[currentDraw]);
	//cmdlist.bindComputeResource(2, particleBuffers[(currentDraw+1) % 3]);

	//currentDraw = (currentDraw + 1) % 3;
	////cout << currentDraw << endl;
	//cmdlist.dispatch(((particleNum / 1024) + 1), 1, 1);



	//cmdlist.resourceTransition(particleBuffers[(currentDraw + 1) % 3], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT,true);
	cmdlist.close();




	render.executeCommands(&compcmdlist);

	render.executeCommands(&cmdlist);
	///fffff
	render.present();
	render.insertSignalFence(comfence, COMMAND_TYPE_COMPUTE);
	render.insertSignalFence(fence);

	render.waitFenceIncreament(fence);
	render.waitFenceIncreament(comfence);

	

	currentDraw = (currentDraw + 1) % 3;

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

int main()
{
	int testnumber = 3;
	windows.initialize(1280, 720, "GPUParticles");
	windows.openWindow();
	glfwSetCursorPosCallback(windows.mWindow, cursor_pos_callback);
	glfwSetMouseButtonCallback(windows.mWindow, mouse_button_callback);
	glfwSetScrollCallback(windows.mWindow, scroll_callback);


	int limit = 10000;
	int count = 0;
	initializeRender();
	loadAsset();
	while (windows.isRunning())
	{

		windows.pollInput();
		update2();
	}
	releaseRender();
	windows.closeWindow();
	windows.termianate();
	return 0;
}