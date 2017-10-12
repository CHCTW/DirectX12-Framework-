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
Pipeline ParticleDraw;
Pipeline ParticleUpdate;
void initializeRender()
{


	//Camera cam;



	render.initialize();
	RenderTargetFormat retformat(true);
	render.createSwapChain(windows, swapChainCount, retformat);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render.mDevice);
	fence.fenceValue = 1;
	fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);

	srvheap.ininitialize(render.mDevice, 1);
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
	camera.updateViewProj();

	ParticleRootsig.mParameters.resize(3);
	ParticleRootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	ParticleRootsig.mParameters[0].mResCounts = 1;
	ParticleRootsig.mParameters[0].mBindSlot = 0;
	ParticleRootsig.mParameters[0].mResource = &sceneBuffer;
	ParticleRootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	ParticleRootsig.mParameters[1].mResCounts = 1;
	ParticleRootsig.mParameters[1].mBindSlot = 0;
	ParticleRootsig.mParameters[2].mType = PARAMETERTYPE_UAV;
	ParticleRootsig.mParameters[2].mResCounts = 1;
	ParticleRootsig.mParameters[2].mBindSlot = 0;
	ParticleRootsig.initialize(render.mDevice);

	
//	ParticleDrawRootSig.mParameters.resize(2);
//	ParticleDrawRootSig.
	ShaderSet ParDrawShader;
	ParDrawShader.shaders[VS].load("Shaders/ParticleDraw.hlsl", "VSMain", VS);
	ParDrawShader.shaders[PS].load("Shaders/ParticleDraw.hlsl", "PSMain", PS);

	ParticleDraw.createGraphicsPipeline(render.mDevice, ParticleRootsig, ParDrawShader, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(),VERTEX_LAYOUT_TYPE_NONE_SPLIT, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);


	ShaderSet ParUpdateShader;
	ParUpdateShader.shaders[CS].load("Shaders/ParticleUpdate.hlsl", "CSMain", CS);

	ParticleUpdate.createComputePipeline(render.mDevice, ParticleRootsig, ParUpdateShader);

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
	cmdlist.resourceBarrier(vertexBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(normalBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.updateBufferData(vertexBuffer, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(normalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.resourceBarrier(vertexBuffer.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(normalBuffer.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	cmdlist.bindPipeline(InitPar);
	cmdlist.bindDescriptorHeaps(&srvheap);
	cmdlist.bindComputeRootSigature(ParticleRootsig);
	//cmdlist.bindGraphicsRootSigature(ParticleRootsig);


	cmdlist.bindComputeResource(2, particleBuffers[0]);
	cmdlist.dispatch((particleNum / 128) + 1, 1, 1);
//	cmdlist.resourceBarrier(particleBuffers[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceBarrier(particleBuffers[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceBarrier(particleBuffers[2], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);







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

	InitPar.release();
	//	import.FreeScene();
}

void releaseRender()
{
	ParticleUpdate.release();
	ParticleDraw.release();
	ParticleRootsig.realease();
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
	cmdlist.bindDescriptorHeaps(&srvheap);
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.renderTargetBarrier(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	cmdlist.resourceBarrier(particleBuffers[currentDraw], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceBarrier(particleBuffers[(currentDraw+1)%3], D3D12_RESOURCE_STATE_GENERIC_READ,D3D12_RESOURCE_STATE_UNORDERED_ACCESS);


	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(render.mSwapChainRenderTarget[frameIndex]);
	//cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//cmdlist.bindIndexBuffer(indexBuffer);
	//cmdlist.bindVertexBuffers(vertexBuffer, normalBuffer);
	//cmdlist.drawIndexedInstanced(mesh->mNumFaces * 3, 1, 0, 0);


	cmdlist.bindGraphicsRootSigature(ParticleRootsig);
	cmdlist.bindPipeline(ParticleDraw);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	cmdlist.bindGraphicsResource(0, cameraBuffer);
	cmdlist.bindGraphicsResource(1, particleBuffers[currentDraw]);
	cmdlist.bindGraphicsResource(2, particleBuffers[(currentDraw+1)%3]);
	cmdlist.drawInstance(particleNum,1, 0, 0);

	cmdlist.bindPipeline(ParticleUpdate);
	cmdlist.bindComputeRootSigature(ParticleRootsig);
	cmdlist.bindComputeResource(0, sceneBuffer);
	cmdlist.bindComputeResource(1, particleBuffers[currentDraw]);
	cmdlist.bindComputeResource(2, particleBuffers[(currentDraw+1) % 3]);

	currentDraw = (currentDraw + 1) % 3;
	//cout << currentDraw << endl;
	cmdlist.dispatch(((particleNum / 1024) + 1), 1, 1);




	cmdlist.renderTargetBarrier(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
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
	windows.initialize(1600, 900, "GPUParticles");
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
		update();
	}
	releaseRender();
	windows.closeWindow();
	windows.termianate();
	return 0;
}