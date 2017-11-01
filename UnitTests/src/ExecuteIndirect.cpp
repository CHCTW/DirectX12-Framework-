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
struct IndirectCommand {
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
struct InstanceData  // for jump GPU address for const buffer, need to alighment with 256byte
{
	Matrices transform; // 128 byte
	float minx;
	float miny;
	float minz; //12 byte
	float maxx;
	float maxy;
	float maxz; //12 byte
	float r;
	float g;
	float b;  // 12 byte
	float padding[23];//92 byte

};

unsigned int xnum = 20;
unsigned int ynum = 20;
unsigned int znum = 20;
float edgeleng = 100;
unsigned int objnum ;
Buffer indstanceConstBuffer;
Buffer indirectBuffer;
std::vector<InstanceData> instanceInf;
std::vector<IndirectCommand> commandInf;
CommandSignature cmdSig;
void initializeRender()
{


	//Camera cam;

	//cout << sizeof(teststruct) << endl;
	//cout << sizeof(DrawIndexedArgument) << endl;
	//cout << sizeof(IndirectCommand) << endl;
	//system("pause");

	render.initialize();
	RenderTargetFormat retformat(true);
	render.createSwapChain(windows, swapChainCount, retformat);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render.mDevice);
	fence.fenceValue = 1;
	fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);

	srvheap.ininitialize(render.mDevice, 1);
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

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.updateViewProj();
}

void loadAsset()
{



	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();







	import.ReadFile("Assets/teapot.obj", aiProcessPreset_TargetRealtime_Quality);
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

	if (mesh->HasTangentsAndBitangents())
		cout << "Has tangent" << endl;


	objnum = xnum*ynum*znum;


	indstanceConstBuffer.createConstantBuffer(render.mDevice, sizeof(InstanceData)*objnum);
	indirectBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ);

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
				t.CacNewTransform();
				instanceInf[i*ynum*znum + j*znum + k].transform = t.getMatrices();
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

	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceBarrier(vertexBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(normalBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.resourceBarrier(indirectBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.updateBufferData(vertexBuffer, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(normalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indirectBuffer, commandInf.data(), sizeof(IndirectCommand)*commandInf.size());

	cmdlist.resourceBarrier(indirectBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceBarrier(vertexBuffer.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(normalBuffer.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
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
	cmdSig.release();
	indirectBuffer.release();
	indstanceConstBuffer.release();
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
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex]);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(render.mSwapChainRenderTarget[frameIndex]);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist.bindIndexBuffer(indexBuffer);
	cmdlist.bindVertexBuffers(vertexBuffer, normalBuffer);
	//cmdlist.drawIndexedInstanced(mesh->mNumFaces * 3, 1, 0, 0);
	cmdlist.executeIndirect(cmdSig, objnum, indirectBuffer, 0);
//	cmdlist.executeIndirect(cmdSig, 1, indirectBuffer, sizeof(IndirectCommand));

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
	windows.initialize(1600, 900, "ExecuteIndirect");
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