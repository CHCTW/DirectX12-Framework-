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
#include <limits>
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
Buffer vertexBuffer;
Buffer normalBuffer;
Buffer indexBuffer;
Buffer cameraBuffer;
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
DescriptorHeap srvheap;

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
struct IndirectCommand {  // a 4 byte padding will automaticly hide in the end , due to 8 byte alignment
	GpuAddress  cbvaddress;
	DrawIndexedArgument draw;
};
struct teststruct {
	DrawIndexedArgument draw2;
	DrawIndexedArgument draw3;
	DrawIndexedArgument draw4;
	GpuAddress  cbvaddress;
	int test;
	DrawIndexedArgument draw5;
};
struct float3 {
	float x;
	float y;
	float z;
};
struct InstanceData  // for jump GPU address for const buffer, need to alighment with 256byte
{
	Matrices transform; // 128 byte
	float3 min;
	float padding;
	float3 max;
	float padding2;
	float r;
	float g;
	float b;  // 12 byte
	float paddingend[21];//92 byte

};

unsigned int xnum = 20;
unsigned int ynum = 20;
unsigned int znum = 20;
float edgeleng = 100;
unsigned int objnum ;
Buffer indstanceConstBuffer;
Buffer instanceStructBuffer; // use for frustum culliing
Buffer indirectBuffer;
Buffer cullIndirectBuffer[3]; // use for culled execute indirect
std::vector<InstanceData> instanceInf;
std::vector<IndirectCommand> commandInf;
CommandSignature cmdSig;
RootSignature cullrootsig;
Pipeline cullPipeline;
SpecCamera camera2;
Buffer cameraBuffer2;
bool maincamer = true;
void initializeRender()
{


	//Camera cam;

	//cout << sizeof(teststruct) << endl;
	//cout << sizeof(DrawIndexedArgument) << endl;
	//cout << sizeof(IndirectCommand) << endl;
	//system("pause");

	render.initialize();
	RenderTargetFormat retformat(true);
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
	for (int i = 0; i < swapChainCount; ++i)
	{
		cmdalloc[i].initialize(render.mDevice);
		cmdlist[i].initial(render.mDevice, cmdalloc[i]);
		fence[i].initialize(render);
	}
	
	//fence.fenceValue = 1;
	//fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);

	srvheap.ininitialize(render.mDevice, 1);
	depthBuffer.resize(swapChainCount);
	depthBuffer[0].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[1].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);
	depthBuffer[2].CreateTexture(render, srvheap, retformat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_DSV);


	rootsig.mParameters.resize(2);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;
	rootsig.mParameters[1].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[1].mTable = false;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 1;
	rootsig.mParameters[1].mResource = &indstanceConstBuffer;
	rootsig.initialize(render.mDevice);


	cmdSig.mParameters.resize(2);
	cmdSig.mParameters[0].ConstantBufferView.RootParameterIndex = 1;
	cmdSig.mParameters[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	cmdSig.mParameters[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	cmdSig.initialize(render.mDevice, rootsig);


	shaderset.shaders[VS].load("Shaders/ExecuteIndirect.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/ExecuteIndirect.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);



	cullrootsig.mParameters.resize(4);
	cullrootsig.mParameters[0].mType = PARAMETERTYPE_UAV;
	cullrootsig.mParameters[0].mResCounts = 1;
	cullrootsig.mParameters[0].mBindSlot = 0;
	cullrootsig.mParameters[1].mType = PARAMETERTYPE_CBV;
	cullrootsig.mParameters[1].mResCounts = 1;
	cullrootsig.mParameters[1].mBindSlot = 0;
	cullrootsig.mParameters[1].mResource = &cameraBuffer;
	cullrootsig.mParameters[2].mType = PARAMETERTYPE_SRV; // all indirect command buffer
	cullrootsig.mParameters[2].mResCounts = 1;
	cullrootsig.mParameters[2].mBindSlot = 0;
	cullrootsig.mParameters[2].mResource = &indirectBuffer;
	cullrootsig.mParameters[3].mType = PARAMETERTYPE_SRV; // instaned data
	cullrootsig.mParameters[3].mResCounts = 1;
	cullrootsig.mParameters[3].mBindSlot = 1;
	cullrootsig.mParameters[3].mResource = &instanceStructBuffer;
	cullrootsig.initialize(render.mDevice);

	ShaderSet cullshader;
	cullshader.shaders[CS].load("Shaders/FrsutumCulling.hlsl", "CSMain", CS);
	cullPipeline.createComputePipeline(render.mDevice, cullrootsig, cullshader);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.updateViewProj();


	camera2.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera2.updateViewProj();

}

void loadAsset()
{



	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();

	cameraBuffer2.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer2.maptoCpu();





	import.ReadFile("Assets/sphere.obj", aiProcessPreset_TargetRealtime_Quality);
	scene = import.GetScene();
	mesh = scene->mMeshes[0];
	mesh->HasPositions();
	UINT verticnum = mesh->mNumVertices;

	if (!vertexBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float)))
	{
		cout << "Fail for create" << endl;
	}
	normalBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));

	// genrate min,max for AABB box

	float3 max, min;
	max.x = std::numeric_limits<float>::min();
	max.y = std::numeric_limits<float>::min();
	max.z = std::numeric_limits<float>::min();
	min.x = std::numeric_limits<float>::max();
	min.y = std::numeric_limits<float>::max();
	min.z = std::numeric_limits<float>::max();
	for (int i = 0; i < mesh->mNumVertices ; i++) // find min, max
	{
		max.x = std::max(mesh->mVertices[i].x, max.x);
		max.y = std::max(mesh->mVertices[i].y, max.y);
		max.z = std::max(mesh->mVertices[i].z, max.z);

		min.x = std::min(mesh->mVertices[i].x, min.x);
		min.y = std::min(mesh->mVertices[i].y, min.y);
		min.z = std::min(mesh->mVertices[i].z, min.z);
	}

	//cout << max.x << "   " << max.y << "   " << max.z << endl;
	//cout << min.x << "   " << min.y << "   " << min.z << endl;

	indexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * 3 * mesh->mNumFaces);

	std::vector<unsigned int> indexdata;
	indexdata.resize(mesh->mNumFaces * 3);
	cout << mesh->mNumFaces << endl;
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		indexdata[i * 3] = mesh->mFaces[i].mIndices[0];
		indexdata[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		indexdata[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}

	if (mesh->HasTangentsAndBitangents())
		cout << "Has tangent" << endl;


	objnum = xnum*ynum*znum;


	indstanceConstBuffer.createConstantBuffer(render.mDevice, sizeof(InstanceData)*objnum); // not register in desricptor heap, so can have larget size const buffer
	instanceStructBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstanceData), objnum, STRUCTERED_BUFFER_TYPE_READ); // use for culling, start to thinking that with dynamic indexing. do we really need to use const buffer for instant data?

	indirectBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ);
	cullIndirectBuffer[0].createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE,true);
	cullIndirectBuffer[1].createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, true);
	cullIndirectBuffer[2].createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, true);

	instanceInf.resize(objnum);
	float xoffset = edgeleng / (float)(xnum + 1);
	float yoffset = edgeleng / (float)(ynum + 1);
	float zoffset = edgeleng / (float)(znum + 1);
	float x = -edgeleng / 2 + xoffset;
	float y = -edgeleng / 2 + yoffset;
	float z = -edgeleng / 2 + zoffset;
	for (int i = 0; i < xnum; ++i)
	{
		y = -edgeleng / 2 + yoffset;
		for (int j = 0; j < ynum; ++j)
		{
			z = -edgeleng / 2 + zoffset;
			for (int k = 0; k < znum; ++k)
			{
				Transform t;
				t.setPosition(x, y, z);
				t.setScale(2, 1, 3);
				t.addAngle(glm::vec3(0, 45, 45));
				t.CacNewTransform();
				instanceInf[i*ynum*znum + j*znum + k].transform = t.getMatrices();
				instanceInf[i*ynum*znum + j*znum + k].max = max;
				instanceInf[i*ynum*znum + j*znum + k].min = min;
				z += zoffset;
			}
			y += yoffset;
		}
		x += xoffset;
		//instanceInf[i].transform
	}
	indstanceConstBuffer.maptoCpu();
	indstanceConstBuffer.updateBufferfromCpu(instanceInf.data(), objnum * sizeof(InstanceData));

	GpuAddress cbvaddress = indstanceConstBuffer.GpuAddress;
	IndirectCommand command;
	command.draw.InstanceCount = 1;
	command.draw.StartInstanceLocation = 0;
	command.draw.IndexCountPerInstance = mesh->mNumFaces * 3;
	command.draw.StartIndexLocation = 0;
	command.draw.BaseVertexLocation = 0;

	for (int i = 0; i < objnum; ++i)  // fill the indirect buffer
	{
	
		command.cbvaddress = cbvaddress;
		command.draw.StartInstanceLocation = 0;
		command.draw.InstanceCount = 1;
		command.draw.IndexCountPerInstance = mesh->mNumFaces * 3;
		command.draw.StartIndexLocation = 0;
		command.draw.BaseVertexLocation = 0;
		commandInf.push_back(command);
		cbvaddress += sizeof(InstanceData);
		
	}
	//cout << commandInf.size() << endl;

	cmdalloc[0].reset();
	cmdlist[0].reset(Pipeline());
	cmdlist[0].resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist[0].resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist[0].resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist[0].resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist[0].resourceTransition(indirectBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist[0].resourceTransition(instanceStructBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);


	cmdlist[0].updateBufferData(vertexBuffer, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist[0].updateBufferData(indexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist[0].updateBufferData(normalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist[0].updateBufferData(indirectBuffer, commandInf.data(), sizeof(IndirectCommand)*commandInf.size());

	cmdlist[0].updateBufferData(instanceStructBuffer, instanceInf.data(), objnum * sizeof(InstanceData));


	cmdlist[0].resourceTransition(instanceStructBuffer,D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist[0].resourceTransition(indirectBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist[0].resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist[0].resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist[0].resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	cmdlist[0].resourceTransition(cullIndirectBuffer[0], D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdlist[0].resourceTransition(cullIndirectBuffer[1], D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdlist[0].resourceTransition(cullIndirectBuffer[2], D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,true);
	cmdlist[0].close();
	render.executeCommands(&cmdlist[0]);
	render.insertSignalFence(fence[0]);
	render.waitFenceIncreament(fence[0]);
	//const UINT64 fenval = fence.fenceValue;
	//render.mCommandQueue->Signal(fence.mDx12Fence, fenval);
	//fence.fenceValue++;

	//if (fence.mDx12Fence->GetCompletedValue() < fenval)
	//{
	//	fence.mDx12Fence->SetEventOnCompletion(fenval, fenceEvet);
	//	WaitForSingleObject(fenceEvet, INFINITE);
	//}
	//	import.FreeScene();
}

void releaseRender()
{
	render.waitFenceIncreament(fence[0]);
	render.waitFenceIncreament(fence[1]);
	render.waitFenceIncreament(fence[2]);

	depthBuffer[2].release();
	depthBuffer[1].release();
	depthBuffer[0].release();
	cullPipeline.release();
	cullrootsig.realease();
	cullIndirectBuffer[2].release();
	cullIndirectBuffer[1].release();
	cullIndirectBuffer[0].release();
	instanceStructBuffer.release();
	cmdSig.release();
	indirectBuffer.release();
	indstanceConstBuffer.release();
	import.FreeScene();
	cameraBuffer2.release();
	cameraBuffer.release();
	indexBuffer.release();
	normalBuffer.release();
	srvheap.release();
	pipeline.release();
	rootsig.realease();
	vertexBuffer.release();
	for (int i = 0; i < swapChainCount; ++i)
	{
		fence[i].release();
		cmdlist[i].release();
		cmdalloc[i].release();
	}
	render.releaseSwapChain();
	render.release();
	CloseHandle(fenceEvet);
}

void update()
{
	camera.updateViewProj();
	camera2.updateViewProj();
	cameraBuffer.updateBufferfromCpu(camera.getMatrix(), sizeof(ViewProjection));
	cameraBuffer2.updateBufferfromCpu(camera2.getMatrix(), sizeof(ViewProjection));
	frameIndex = render.getCurrentSwapChainIndex();
	render.waitFenceIncreament(fence[frameIndex]);
	cmdalloc[frameIndex].reset();
	cmdlist[frameIndex].reset(cullPipeline);
	cmdlist[frameIndex].bindDescriptorHeaps(&srvheap);
	// reset the command number and start culling
	cmdlist[frameIndex].resourceTransition(cullIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_COPY_DEST,true); //reset count
	cmdlist[frameIndex].setCounterforStructeredBuffer(cullIndirectBuffer[frameIndex], 0);
	cmdlist[frameIndex].resourceTransition(cullIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);
	cmdlist[frameIndex].bindComputeRootSigature(cullrootsig);
	cmdlist[frameIndex].bindComputeResource(0, cullIndirectBuffer[frameIndex]);
	cmdlist[frameIndex].dispatch((objnum + 1023) / 1024, 1, 1);
	cmdlist[frameIndex].resourceTransition(cullIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,true);


	cmdlist[frameIndex].bindPipeline(pipeline);
	cmdlist[frameIndex].bindGraphicsRootSigature(rootsig);
	cmdlist[frameIndex].setViewPort(viewport);
	cmdlist[frameIndex].setScissor(scissor);
	if (maincamer)
		cmdlist[frameIndex].bindGraphicsResource(0, cameraBuffer);
	else
		cmdlist[frameIndex].bindGraphicsResource(0, cameraBuffer2);
	cmdlist[frameIndex].swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	cmdlist[frameIndex].bindRenderTarget(render.mSwapChainRenderTarget[frameIndex],depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	cmdlist[frameIndex].clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist[frameIndex].clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist[frameIndex].setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist[frameIndex].bindIndexBuffer(indexBuffer);
	cmdlist[frameIndex].bindVertexBuffers(vertexBuffer, normalBuffer);
//	cmdlist.executeIndirect(cmdSig, objnum, indirectBuffer, 0);



	cmdlist[frameIndex].executeIndirect(cmdSig, objnum, cullIndirectBuffer[frameIndex], 0, cullIndirectBuffer[frameIndex], cullIndirectBuffer[frameIndex].mBufferSize - sizeof(UINT));

	cmdlist[frameIndex].swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT,true);
	cmdlist[frameIndex].close();
	render.executeCommands(&cmdlist[frameIndex]);
	render.present();
	render.insertSignalFence(fence[frameIndex]);
	render.waitFenceIncreament(fence[frameIndex]);

	//const UINT64 fenval = fence.fenceValue;
	//render.mCommandQueue->Signal(fence.mDx12Fence, fenval);
	//fence.fenceValue++;

	//if (fence.mDx12Fence->GetCompletedValue() < fenval)
	//{
	//	fence.mDx12Fence->SetEventOnCompletion(fenval, fenceEvet);
	//	WaitForSingleObject(fenceEvet, INFINITE);
	//}
}


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	curxoffet = xpos - curxpos;
	curyoffet = ypos - curypos;

	curxpos = xpos;
	curypos = ypos;

	if (press)
	{
		if(maincamer)
			camera.addAngle(curxoffet / 5.0, curyoffet / 5.0);
		else
			camera2.addAngle(curxoffet / 5.0, curyoffet / 5.0);
	}
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
	{
		if (maincamer)
			camera.addZoom(1);
		else
			camera2.addZoom(1);
	}
	if (yoffset == -1)
	{
		if (maincamer)
			camera.addZoom(-1);
		else
			camera2.addZoom(-1);
	}
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{


	if (key == GLFW_KEY_1)
		maincamer = true;
	if (key == GLFW_KEY_2)
		maincamer = false;
}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "ExecuteIndirect");
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