#include <iostream>
#include <vector>
#include <algorithm>
#include <assimp\ai_assert.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\mesh.h>
#include <chrono>
#include "Window.h"
#include "Render.h"
#include "StructureHeaders.h"
#include "SpecCamera.h"
#include "Transform.h"
#include "Material.h"
#include "SpotLight.h"
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
Assimp::Importer import;
aiScene const * scene = nullptr;
aiMesh* mesh = nullptr;

Assimp::Importer buddhaimport;
Assimp::Importer groundimport;

SpecCamera camera;
float curxpos;
float curypos;
float curxoffet;
float curyoffet;
bool press = false;


ObjectData Bunnys;

const UINT rowcount = 3;
const UINT collomcount = 3;
UINT spherecount = rowcount*collomcount;

float radious = 8;
float heightgap = 1.5;
float rotationoffset = 0.0;
float radian;
float rotationspeed = 0.0005f;


bool lightmove;
ObjectData Buddha;
ObjectData Ground;
SpotLight light;

std::chrono::high_resolution_clock::time_point pre;

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

	rootsig.mParameters.resize(4);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;
	rootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 0;
	rootsig.mParameters[1].mResource = &Bunnys.mStructeredBuffer;
	rootsig.mParameters[2].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[2].mResCounts = 1;
	rootsig.mParameters[2].mBindSlot = 1;
	rootsig.mParameters[2].mResource = &lightBuffer;
	rootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[3].mType = PARAMETERTYPE_ROOTCONSTANT;
	rootsig.mParameters[3].mResCounts = 1;
	rootsig.mParameters[3].mBindSlot = 2;
	rootsig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_ALL;


	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/GeometryShader.hlsl", "VSMain", VS);
	shaderset.shaders[GS].load("Shaders/GeometryShader.hlsl", "GSMain", GS);
	shaderset.shaders[PS].load("Shaders/GeometryShader.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT), VERTEX_LAYOUT_TYPE_SPLIT_ALL);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.updateViewProj();
}

void loadAsset()
{


	camera.mZoom = 15;

	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();

	lightBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(SpotLightData));
	lightBuffer.maptoCpu();

	light.setRadius(100);
	light.setColor(100, 100, 100);

	buddhaimport.ReadFile("Assets/sphere.obj", aiProcessPreset_TargetRealtime_Quality);

	aiMesh* buddha = buddhaimport.GetScene()->mMeshes[0];
	Buddha.mVertexBufferData.createVertexBuffer(render.mDevice, buddha->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	Buddha.mNormalBuffer.createVertexBuffer(render.mDevice, buddha->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	std::vector<unsigned int> buddhaindexdata;
	buddhaindexdata.resize(buddha->mNumFaces * 3);
	for (int i = 0; i < buddha->mNumFaces; i++)
	{
		buddhaindexdata[i * 3] = buddha->mFaces[i].mIndices[0];
		buddhaindexdata[i * 3 + 1] = buddha->mFaces[i].mIndices[1];
		buddhaindexdata[i * 3 + 2] = buddha->mFaces[i].mIndices[2];
	}
	Buddha.mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * buddha->mNumFaces * 3);
	Buddha.indexCount = buddha->mNumFaces * 3;
	Buddha.mNum = 1;
	Buddha.mBufferData.resize(Buddha.mNum);
	Buddha.mPosition.resize(Buddha.mNum);
	Buddha.mStructeredBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Buddha.mNum, STRUCTERED_BUFFER_TYPE_READ);
	Buddha.mPosition[0].setAngle(0, 0, 0);
	Buddha.mPosition[0].setScale(3, 3, 3);
	Buddha.mPosition[0].setPosition(0, 0, 0);
	Buddha.mPosition[0].CacNewTransform();
	Buddha.mBufferData[0].mMatrices = Buddha.mPosition[0].getMatrices();
	Buddha.mBufferData[0].mMaterial.mAlbedo = glm::vec3(1.00, 0.71, 0.29);
	Buddha.mBufferData[0].mMaterial.mRoughness = 0.4;
	Buddha.mBufferData[0].mMaterial.mMetallic = 1.0;


	groundimport.ReadFile("Assets/insidecube.obj", aiProcessPreset_TargetRealtime_Fast);
	aiMesh* ground = groundimport.GetScene()->mMeshes[0];
	Ground.mVertexBufferData.createVertexBuffer(render.mDevice, ground->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	Ground.mNormalBuffer.createVertexBuffer(render.mDevice, ground->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	std::vector<unsigned int> groundindexdata;
	groundindexdata.resize(ground->mNumFaces * 3);
	for (int i = 0; i < ground->mNumFaces; i++)
	{
		groundindexdata[i * 3] = ground->mFaces[i].mIndices[0];
		groundindexdata[i * 3 + 1] = ground->mFaces[i].mIndices[1];
		groundindexdata[i * 3 + 2] = ground->mFaces[i].mIndices[2];
	}
	Ground.mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * ground->mNumFaces * 3);
	Ground.indexCount = ground->mNumFaces * 3;
	Ground.mNum = 1;
	Ground.mBufferData.resize(Ground.mNum);
	Ground.mPosition.resize(Ground.mNum);
	Ground.mStructeredBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Ground.mNum, STRUCTERED_BUFFER_TYPE_READ);
	//Ground.mPosition[0].setAngle(-90, 0, 0);
	Ground.mPosition[0].setScale(10, 10, 10);
	Ground.mPosition[0].setPosition(0, 0, 0);
	Ground.mPosition[0].CacNewTransform();
	Ground.mBufferData[0].mMatrices = Ground.mPosition[0].getMatrices();
	Ground.mBufferData[0].mMaterial.mAlbedo = glm::vec3(0.55, 0.55, 0.55);
	Ground.mBufferData[0].mMaterial.mRoughness = 0.8;
	Ground.mBufferData[0].mMaterial.mMetallic = 0.8;





	// load sphere data

	import.ReadFile("Assets/bunny.obj", aiProcessPreset_TargetRealtime_Quality);
	scene = import.GetScene();
	mesh = scene->mMeshes[0];

	UINT verticnum = mesh->mNumVertices;

	if (!Bunnys.mVertexBufferData.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float)))
	{
		cout << "Fail for create" << endl;
	}
	Bunnys.mNormalBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));


	Bunnys.mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * 3 * mesh->mNumFaces);

	std::vector<unsigned int> indexdata;
	indexdata.resize(mesh->mNumFaces * 3);
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		indexdata[i * 3] = mesh->mFaces[i].mIndices[0];
		indexdata[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		indexdata[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}

	// initialize spheres position data
	Bunnys.mNum = spherecount;
	Bunnys.indexCount = mesh->mNumFaces * 3;
	Bunnys.mBufferData.resize(Bunnys.mNum);
	Bunnys.mPosition.resize(Bunnys.mNum);

	Bunnys.mStructeredBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Bunnys.mNum, STRUCTERED_BUFFER_TYPE_READ);

	radian = 2 * 3.14159f / rowcount;
	float rough = 1.0f / (rowcount - 1);
	float metalic = 1.0f / (collomcount - 1);
	for (int i = 0; i < Bunnys.mNum; ++i)
	{
		int height = i / rowcount;
		int rowpos = i % rowcount;
		Bunnys.mPosition[i].setPosition(radious*cos(i*radian), heightgap*height, radious*sin(i*radian));
		//Bunnys.mPosition[i].setAngle(-90, 0, 0);
		Bunnys.mPosition[i].CacNewTransform();
		Bunnys.mBufferData[i].mMatrices = Bunnys.mPosition[i].getMatrices();
		Bunnys.mBufferData[i].mMaterial.mAlbedo = glm::vec3(1.0, 0.0, 0.0);
		Bunnys.mBufferData[i].mMaterial.mRoughness = (rowpos)*rough;
		Bunnys.mBufferData[i].mMaterial.mMetallic = height*metalic;

	}




	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(Bunnys.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Bunnys.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Bunnys.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);


	cmdlist.updateBufferData(Bunnys.mVertexBufferData, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Bunnys.mIndexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Bunnys.mNormalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Bunnys.mStructeredBuffer, Bunnys.mBufferData.data(), Bunnys.mNum * sizeof(InstancedInformation));

	cmdlist.updateBufferData(Buddha.mVertexBufferData, buddha->mVertices, buddha->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Buddha.mIndexBuffer, buddhaindexdata.data(), buddha->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Buddha.mNormalBuffer, buddha->mNormals, buddha->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Buddha.mStructeredBuffer, Buddha.mBufferData.data(), Buddha.mNum * sizeof(InstancedInformation));

	cmdlist.updateBufferData(Ground.mVertexBufferData, ground->mVertices, ground->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Ground.mIndexBuffer, groundindexdata.data(), ground->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Ground.mNormalBuffer, ground->mNormals, ground->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Ground.mStructeredBuffer, Ground.mBufferData.data(), Ground.mNum * sizeof(InstancedInformation));

	cmdlist.resourceTransition(Bunnys.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Bunnys.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Bunnys.mIndexBuffer,D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceTransition(Buddha.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ,true);


	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.insertSignalFence(fence);
	render.waitFenceIncreament(fence);
	//	import.FreeScene();

	pre = std::chrono::high_resolution_clock::now();
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
	Buddha.mNormalBuffer.release();
	Buddha.mVertexBufferData.release();
	Buddha.mStructeredBuffer.release();
	Buddha.mIndexBuffer.release();
	Bunnys.mStructeredBuffer.release();
	import.FreeScene();
	cameraBuffer.release();
	Bunnys.mIndexBuffer.release();
	Bunnys.mVertexBufferData.release();
	srvheap.release();
	pipeline.release();
	rootsig.realease();
	Bunnys.mNormalBuffer.release();
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
	rotationoffset += rotationspeed;
	for (int i = 0; i < Bunnys.mNum; ++i)
	{
		int height = i / rowcount;
		int rowpos = i % rowcount;
		Bunnys.mPosition[i].setPosition(radious*cos(rowpos*radian + rotationoffset), heightgap*height - (collomcount*heightgap) / 2, radious*sin(rowpos*radian + rotationoffset));

		//	Bunnys.mPosition[i].setPosition(rowpos*1.5, height, 5);
		//Bunnys.mPosition[i].setAngle(0, (rowpos*radian+ rotationoffset)/3.14159*180, 0);
		Bunnys.mPosition[i].CacNewTransform();
		Bunnys.mBufferData[i].mMatrices = Bunnys.mPosition[i].getMatrices();
	}

}
void onrender()
{
	std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> delta = t - pre;

	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(pipeline);





	cmdlist.resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.updateBufferData(Bunnys.mStructeredBuffer, Bunnys.mBufferData.data(), Bunnys.mNum * sizeof(InstancedInformation));
	cmdlist.resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ,true);











	cmdlist.bindDescriptorHeaps(&srvheap);
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex],depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	float time = delta.count();
	cmdlist.bindGraphicsConstant(3, &time);

	cmdlist.bindIndexBuffer(Bunnys.mIndexBuffer);
	cmdlist.bindVertexBuffers(Bunnys.mVertexBufferData, Bunnys.mNormalBuffer);
	cmdlist.drawIndexedInstanced(Bunnys.indexCount, Bunnys.mNum, 0, 0);

	cmdlist.bindGraphicsResource(1, Buddha.mStructeredBuffer);
	cmdlist.bindIndexBuffer(Buddha.mIndexBuffer);
	cmdlist.bindVertexBuffers(Buddha.mVertexBufferData, Buddha.mNormalBuffer);
	cmdlist.drawIndexedInstanced(Buddha.indexCount, Buddha.mNum, 0, 0);


	//cmdlist.bindGraphicsResource(1, Ground.mStructeredBuffer);
	//cmdlist.bindIndexBuffer(Ground.mIndexBuffer);
	//cmdlist.bindVertexBuffers(Ground.mVertexBufferData, Ground.mNormalBuffer);
	//cmdlist.drawIndexedInstanced(Ground.indexCount, Ground.mNum, 0, 0);




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
	windows.initialize(1600, 900, "Geometry Shader");
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