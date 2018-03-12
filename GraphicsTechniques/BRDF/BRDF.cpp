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
static const uint32_t swapChainCount = 3;
CommandAllocator cmdalloc[swapChainCount];
CommandList cmdlist[swapChainCount];
Window windows;
Pipeline pipeline;
Fence fence[swapChainCount];
HANDLE fenceEvet;
UINT frameIndex;
Buffer cameraBuffer;
Buffer lightBuffer;
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
DescriptorHeap srvheap;

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
Buffer bunnyTrans[swapChainCount];

const UINT rowcount = 10;
const UINT collomcount = 5;
UINT spherecount = rowcount*collomcount;

float radious = 5 ;
float heightgap = 1.5;
float rotationoffset = 0.0;
float radian;
float rotationspeed = 0.0005f;


bool lightmove;
ObjectData Buddha;
ObjectData Ground;
SpotLight light;

void initializeRender()
{


	//Camera cam;



	render.initialize();
	RenderTargetFormat retformat(true);
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
	for (int i = 0; i < swapChainCount; ++i)
	{
		cmdalloc[i].initialize(render.mDevice);
		cmdlist[i].initial(render.mDevice, cmdalloc[i]);
		fence[i].initialize(render);
	}

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
//	rootsig.mParameters[1].mResource = &Bunnys.mStructeredBuffer;
	rootsig.mParameters[2].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[2].mResCounts = 1;
	rootsig.mParameters[2].mBindSlot = 1;
	rootsig.mParameters[2].mResource = &lightBuffer;
	rootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/BRDF.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/BRDF.hlsl", "PSMain", PS);


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

	light.setIntensity(500);
	light.setRadius(1500);
	light.setConeAngle(60.0);
	light.setColor(1, 1, 1);

	buddhaimport.ReadFile("Assets/buddha.obj", aiProcessPreset_TargetRealtime_Quality);
	
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
	Buddha.mPosition[0].setScale(1,1,1);
	Buddha.mPosition[0].setPosition(0, -5, 0);
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
	Ground.mPosition[0].setScale(20, 20, 20);
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
	bunnyTrans[0].createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Bunnys.mNum, STRUCTERED_BUFFER_TYPE_READ);
	bunnyTrans[2].createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Bunnys.mNum, STRUCTERED_BUFFER_TYPE_READ);
	bunnyTrans[1].createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Bunnys.mNum, STRUCTERED_BUFFER_TYPE_READ);


	radian = 2*3.14159f/ rowcount;
	float rough = 1.0f / (rowcount-1);
	float metalic = 1.0f / (collomcount-1);
	for (int i = 0; i < Bunnys.mNum; ++i)
	{
		int height = i / rowcount;
		int rowpos = i % rowcount;
		Bunnys.mPosition[i].setPosition(radious*cos(i*radian),heightgap*height, radious*sin(i*radian));
		//Bunnys.mPosition[i].setAngle(-90, 0, 0);
		Bunnys.mPosition[i].CacNewTransform();
		Bunnys.mBufferData[i].mMatrices = Bunnys.mPosition[i].getMatrices();
		Bunnys.mBufferData[i].mMaterial.mAlbedo = glm::vec3(1.0, 0.0, 0.0);
		Bunnys.mBufferData[i].mMaterial.mRoughness = (rowpos)*rough;
		Bunnys.mBufferData[i].mMaterial.mMetallic = height*metalic;

	}




	cmdalloc[0].reset();
	cmdlist[0].reset(Pipeline());
	cmdlist[0].resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist[0].resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist[0].resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist[0].resourceTransition(Bunnys.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Bunnys.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Bunnys.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Buddha.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist[0].resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);


	cmdlist[0].updateBufferData(Bunnys.mVertexBufferData, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist[0].updateBufferData(Bunnys.mIndexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist[0].updateBufferData(Bunnys.mNormalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist[0].updateBufferData(Bunnys.mStructeredBuffer, Bunnys.mBufferData.data(), Bunnys.mNum * sizeof(InstancedInformation));

	cmdlist[0].updateBufferData(Buddha.mVertexBufferData, buddha->mVertices, buddha->mNumVertices * 3 * sizeof(float));
	cmdlist[0].updateBufferData(Buddha.mIndexBuffer, buddhaindexdata.data(), buddha->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist[0].updateBufferData(Buddha.mNormalBuffer, buddha->mNormals, buddha->mNumVertices * 3 * sizeof(float));
	cmdlist[0].updateBufferData(Buddha.mStructeredBuffer, Buddha.mBufferData.data(), Buddha.mNum * sizeof(InstancedInformation));

	cmdlist[0].updateBufferData(Ground.mVertexBufferData, ground->mVertices, ground->mNumVertices * 3 * sizeof(float));
	cmdlist[0].updateBufferData(Ground.mIndexBuffer, groundindexdata.data(), ground->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist[0].updateBufferData(Ground.mNormalBuffer, ground->mNormals, ground->mNumVertices * 3 * sizeof(float));
	cmdlist[0].updateBufferData(Ground.mStructeredBuffer, Ground.mBufferData.data(), Ground.mNum * sizeof(InstancedInformation));

	cmdlist[0].resourceTransition(Bunnys.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist[0].resourceTransition(Bunnys.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist[0].resourceTransition(Bunnys.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist[0].resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist[0].resourceTransition(Buddha.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist[0].resourceTransition(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist[0].resourceTransition(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist[0].resourceTransition(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist[0].resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist[0].resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist[0].resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist[0].resourceTransition(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ,true);


	cmdlist[0].close();
	render.executeCommands(&cmdlist[0]);
	render.insertSignalFence(fence[0]);
	render.waitFenceIncreament(fence[0]);

}

void releaseRender()
{
	render.waitFenceIncreament(fence[0]);
	render.waitFenceIncreament(fence[1]);
	render.waitFenceIncreament(fence[2]);
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
	for (int i = 0; i < swapChainCount; ++i)
	{
		fence[i].release();
		cmdlist[i].release();
		cmdalloc[i].release();
		bunnyTrans[0].release();
		bunnyTrans[1].release();
		bunnyTrans[2].release();
	}
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
		Bunnys.mPosition[i].setPosition(radious*cos(rowpos*radian+ rotationoffset), heightgap*height - (collomcount*heightgap)/2, radious*sin(rowpos*radian +rotationoffset));

	//	Bunnys.mPosition[i].setPosition(rowpos*1.5, height, 5);
		//Bunnys.mPosition[i].setAngle(0, (rowpos*radian+ rotationoffset)/3.14159*180, 0);
		Bunnys.mPosition[i].CacNewTransform();
		Bunnys.mBufferData[i].mMatrices = Bunnys.mPosition[i].getMatrices();
	}

}
void onrender()
{
	
	frameIndex = render.getCurrentSwapChainIndex();
	render.waitFenceIncreament(fence[frameIndex]);
	cmdalloc[frameIndex].reset();
	cmdlist[frameIndex].reset(pipeline);





	cmdlist[frameIndex].resourceTransition(bunnyTrans[frameIndex], D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist[frameIndex].updateBufferData(bunnyTrans[frameIndex], Bunnys.mBufferData.data(), Bunnys.mNum * sizeof(InstancedInformation));
	cmdlist[frameIndex].resourceTransition(bunnyTrans[frameIndex], D3D12_RESOURCE_STATE_GENERIC_READ,true);











	cmdlist[frameIndex].bindDescriptorHeaps(&srvheap);
	cmdlist[frameIndex].bindGraphicsRootSigature(rootsig);
	cmdlist[frameIndex].setViewPort(viewport);
	cmdlist[frameIndex].setScissor(scissor);
	cmdlist[frameIndex].swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	cmdlist[frameIndex].bindRenderTarget(render.mSwapChainRenderTarget[frameIndex],depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist[frameIndex].clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist[frameIndex].clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist[frameIndex].setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist[frameIndex].bindGraphicsResource(1, bunnyTrans[frameIndex]);
	cmdlist[frameIndex].bindIndexBuffer(Bunnys.mIndexBuffer);
	cmdlist[frameIndex].bindVertexBuffers(Bunnys.mVertexBufferData, Bunnys.mNormalBuffer);
	cmdlist[frameIndex].drawIndexedInstanced(Bunnys.indexCount, Bunnys.mNum, 0, 0);

	cmdlist[frameIndex].bindGraphicsResource(1, Buddha.mStructeredBuffer);
	cmdlist[frameIndex].bindIndexBuffer(Buddha.mIndexBuffer);
	cmdlist[frameIndex].bindVertexBuffers(Buddha.mVertexBufferData, Buddha.mNormalBuffer);
	cmdlist[frameIndex].drawIndexedInstanced(Buddha.indexCount, Buddha.mNum, 0, 0);


	cmdlist[frameIndex].bindGraphicsResource(1, Ground.mStructeredBuffer);
	cmdlist[frameIndex].bindIndexBuffer(Ground.mIndexBuffer);
	cmdlist[frameIndex].bindVertexBuffers(Ground.mVertexBufferData, Ground.mNormalBuffer);
	cmdlist[frameIndex].drawIndexedInstanced(Ground.indexCount, Ground.mNum, 0, 0);




	cmdlist[frameIndex].swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT,true);
	cmdlist[frameIndex].close();
	render.executeCommands(&cmdlist[frameIndex]);
	render.present();

	render.insertSignalFence(fence[frameIndex]);
//	render.waitFenceIncreament(fence[frameIndex]);
}


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	curxoffet = xpos - curxpos;
	curyoffet = ypos - curypos;

	curxpos = xpos;
	curypos = ypos;

	if (press && !lightmove)
		camera.addAngle(curxoffet / 5.0, curyoffet / 5.0);
	if(press && lightmove)
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
		lightmove  = true;
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		lightmove = false;
	

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		light.setColor(1.0,0.0,0.0);
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		light.setColor(0.0, 0.0, 1.0);
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		light.setColor(1.0, 1.0, 1.0);
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		light.setColor(0.0, 1.0, 0.0);


}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "BRDF");
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