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
#include "Transform.h"
#include "Material.h"
#include "SpotLight.h"
#include "QuadPatch.h"
struct InstancedInformation
{
	Matrices mMatrices; // model and normal transfrom matrix;
	Material mMaterial;
};
struct ObjectData
{
	std::vector<Transform> mPosition;
	std::vector<InstancedInformation> mBufferData;
	Buffer mVertexBufferData;
	Buffer mNormalBuffer;
	Buffer mStructeredBuffer;
	Buffer mIndexBuffer;
	UINT indexCount;
	UINT mNum;
};

using namespace std;
Render render;
CommandAllocator cmdalloc;
CommandList cmdlist;
Window windows;
Pipeline pipeline;
Fence fence;
HANDLE fenceEvet;
UINT frameIndex;
Buffer cameraBuffer;
Buffer lightBuffer;
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
DescriptorHeap srvheap;
static uint32_t swapChainCount = 3;
vector<Texture> depthBuffer;
ShaderSet shaderset;

aiScene const * scene = nullptr;
aiMesh* mesh = nullptr;


Assimp::Importer groundimport;

SpecCamera camera;
float curxpos;
float curypos;
float curxoffet;
float curyoffet;
bool press = false;




const UINT rowcount = 10;
const UINT collomcount = 5;
UINT spherecount = rowcount*collomcount;

float radious = 5;
float heightgap = 1.5;
float rotationoffset = 0.0;
float radian;
float rotationspeed = 0.0005f;


bool lightmove;
ObjectData Ground;
SpotLight light;

QuadPatch patch;

void initializeRender()
{


	//Camera cam;

	

	render.initialize();
	RenderTargetFormat retformat(true);
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render.mDevice);
	fence.fenceValue = 1;
	fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);

	srvheap.ininitialize(render.mDevice, 1);
	depthBuffer.resize(swapChainCount);
	depthBuffer[0].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[1].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[2].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);


	rootsig.mParameters.resize(3);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;
	rootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 0;
	rootsig.mParameters[1].mResource = &Ground.mStructeredBuffer;
	rootsig.mParameters[2].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[2].mResCounts = 1;
	rootsig.mParameters[2].mBindSlot = 1;
	rootsig.mParameters[2].mResource = &lightBuffer;
	rootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/BezierPlane.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/BezierPlane.hlsl", "PSMain", PS);
	shaderset.shaders[HS].load("Shaders/BezierPlane.hlsl", "HSMain", HS);
	shaderset.shaders[DS].load("Shaders/BezierPlane.hlsl", "DSMain", DS);
//	shaderset.shaders[GS].load("Shaders/BezierPlane.hlsl", "GSMain", GS);


	ShaderSet test;
	

	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_NONE, D3D12_FILL_MODE_SOLID), VERTEX_LAYOUT_TYPE_SPLIT_ALL, D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.updateViewProj();
}

void loadAsset()
{

	/*float patchdata[] =
	{
		-10.0f,0.0,10.0f,1.0f,1.0f,1.0f,
		-10.0f,0.0,10.0f,1.0f,1.0f,1.0f,
		
	};*/
	



	
	camera.mZoom = 20;

	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();

	lightBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(SpotLightData));
	lightBuffer.maptoCpu();

	light.setRadius(1000);
	light.setConeAngle(60);
	light.setIntensity(100000);
	light.setColor(1.0, 1.0, 1.0);
	light.addZoom(20);
	light.addAngle(0, 90);


	
	patch.generatePatch(30, 30, 30, 30, SmoothBezier, PerlinGenerate, 0, 3, 3, 5);
	Ground.mVertexBufferData.createVertexBuffer(render.mDevice, patch.mPosition.size() * sizeof(float), sizeof(float) * 3);
	Ground.mNormalBuffer.createVertexBuffer(render.mDevice, patch.mNormal.size() * sizeof(float), sizeof(float) * 3);
	Ground.mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int)*patch.mIndex.size());
	Ground.indexCount = patch.mIndex.size();
	Ground.mNum = 1;
	Ground.mBufferData.resize(Ground.mNum);
	Ground.mPosition.resize(Ground.mNum);
	Ground.mStructeredBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Ground.mNum, STRUCTERED_BUFFER_TYPE_READ);
	//Ground.mPosition[0].setAngle(-90, 0, 0);
	Ground.mPosition[0].setScale(1, 1, 1);
	Ground.mPosition[0].setPosition(0, -5, 0);
	Ground.mPosition[0].CacNewTransform();
	Ground.mBufferData[0].mMatrices = Ground.mPosition[0].getMatrices();
	Ground.mBufferData[0].mMaterial.mAlbedo = glm::vec3(0.55, 0.55, 0.55);
	Ground.mBufferData[0].mMaterial.mRoughness = 0.8;
	Ground.mBufferData[0].mMaterial.mMetallic = 0.8;






	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);

	cmdlist.updateBufferData(Ground.mVertexBufferData, patch.mPosition.data(), patch.mPosition.size() * sizeof(float));
	cmdlist.updateBufferData(Ground.mIndexBuffer, patch.mIndex.data(), patch.mIndex.size() * sizeof(unsigned int));
	cmdlist.updateBufferData(Ground.mNormalBuffer, patch.mNormal.data(), patch.mNormal.size() * sizeof(float));
	cmdlist.updateBufferData(Ground.mStructeredBuffer, Ground.mBufferData.data(), Ground.mNum * sizeof(InstancedInformation));


	cmdlist.resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ,true);


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
	//	import.FreeScene();
}

void releaseRender()
{
	depthBuffer[2].release();
	depthBuffer[1].release();
	depthBuffer[0].release();
	Ground.mNormalBuffer.release();
	Ground.mVertexBufferData.release();
	Ground.mStructeredBuffer.release();
	Ground.mIndexBuffer.release();
	lightBuffer.release();
	
	
	cameraBuffer.release();
	
	srvheap.release();
	pipeline.release();
	rootsig.realease();
	
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


	light.update();

	lightBuffer.updateBufferfromCpu(light.getLightData(), sizeof(SpotLightData));
	//radian = 2 * 3.14159f / Bunnys.mNum;

}
void onrender()
{

	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(pipeline);

	cmdlist.bindDescriptorHeaps(&srvheap);
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex],depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

	cmdlist.bindGraphicsResource(1, Ground.mStructeredBuffer);



	cmdlist.bindIndexBuffer(Ground.mIndexBuffer);
	cmdlist.bindVertexBuffers(Ground.mVertexBufferData, Ground.mNormalBuffer);
	cmdlist.drawIndexedInstanced(Ground.indexCount, 1, 0, 0);





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


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	curxoffet = xpos - curxpos;
	curyoffet = ypos - curypos;

	curxpos = xpos;
	curypos = ypos;

	if (press && !lightmove)
		camera.addAngle(curxoffet / 5.0, curyoffet / 5.0);
	if (press && lightmove)
		light.addAngle(curxoffet / 5.0, curyoffet / 5.0);
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
	if (!lightmove)
	{
		if (yoffset == 1)
			camera.addZoom(1);
		if (yoffset == -1)
			camera.addZoom(-1);
	}
	else
	{
		if (yoffset == 1)
			light.addZoom(1);
		if (yoffset == -1)
			light.addZoom(-1);
	}
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		lightmove = true;
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		lightmove = false;


	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		light.setColor(100.0, 0.0, 0.0);
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		light.setColor(0.0, 0.0, 100.0);
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		light.setColor(100.0, 100.0, 100.0);
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		light.setColor(0.0, 100.0, 0.0);


}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "BezierPlane");
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
		onrender();
	}
	releaseRender();
	windows.closeWindow();
	windows.termianate();
	return 0;
}