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
struct IndirectCommand {  // a 4 byte padding will automaticly hide in the end , due to 8 byte alignment
	GpuAddress  cbvaddress;
	DrawIndexedArgument draw;
};
struct OccuIndirCommad {  // 
	GpuAddress  cbvaddress;
	unsigned int cmdindex;
	DrawArgument draw;
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
float edgeleng = 50;
unsigned int objnum;
Buffer indstanceConstBuffer;
Buffer instanceStructBuffer; // use for frustum culliing
Buffer indirectBuffer;
Buffer cullIndirectBuffer[3]; // use for culled execute indirect
Buffer occuIndirectBuffer[3]; // use for occu execute indirect
Buffer occResult[3];
std::vector<InstanceData> instanceInf;
std::vector<IndirectCommand> commandInf;
CommandSignature cmdSig;
CommandSignature occCmdSig;
RootSignature cullrootsig;
Pipeline cullPipeline;
SpecCamera camera2;
Buffer cameraBuffer2;
bool useocculusion = true;
QueryHeap occqueheap;
Pipeline occPipeline;
RootSignature occurootsig;
vector<unsigned int> clearbuf;
Pipeline occCmdPipeline;
RootSignature occucomdrootsig;
void initializeRender()
{

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
	cmdSig.mParameters[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	cmdSig.mParameters[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	cmdSig.initialize(render.mDevice, rootsig);

	shaderset.shaders[VS].load("Shaders/ExecuteIndirect.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/ExecuteIndirect.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);


	occurootsig.mParameters.resize(4);
	occurootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	occurootsig.mParameters[0].mResCounts = 1;
	occurootsig.mParameters[0].mBindSlot = 0;
	occurootsig.mParameters[0].mResource = &cameraBuffer;
	occurootsig.mParameters[1].mType = PARAMETERTYPE_CBV;
	occurootsig.mParameters[1].mTable = false;
	occurootsig.mParameters[1].mResCounts = 1;
	occurootsig.mParameters[1].mBindSlot = 1;
	occurootsig.mParameters[1].mResource = &indstanceConstBuffer;
	occurootsig.mParameters[2].mType = PARAMETERTYPE_ROOTCONSTANT;
	occurootsig.mParameters[2].mTable = false;
	occurootsig.mParameters[2].mResCounts = 1;
	occurootsig.mParameters[2].mBindSlot = 2;
	occurootsig.mParameters[3].mType = PARAMETERTYPE_UAV;
	occurootsig.mParameters[3].mResCounts = 1;
	occurootsig.mParameters[3].mBindSlot = 0;
	occurootsig.mParameters[3].mResource = &occResult[0];
	occurootsig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	occurootsig.initialize(render.mDevice);


	occCmdSig.mParameters.resize(3);
	occCmdSig.mParameters[0].ConstantBufferView.RootParameterIndex = 1;
	occCmdSig.mParameters[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	occCmdSig.mParameters[1].Constant.Num32BitValuesToSet = 1;
	occCmdSig.mParameters[1].Constant.RootParameterIndex = 2;
	occCmdSig.mParameters[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	occCmdSig.mParameters[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	occCmdSig.initialize(render.mDevice, occurootsig);

	


	ShaderSet occshader;
	occshader.shaders[VS].load("Shaders/OcculusionBox.hlsl", "VSMain", VS);
	occshader.shaders[PS].load("Shaders/OcculusionBox.hlsl", "PSMain", PS);

	occPipeline.createGraphicsPipeline(render.mDevice, occurootsig, occshader, retformat, DepthStencilState::DepthStencilState(true,false), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);


	cullrootsig.mParameters.resize(5);
	cullrootsig.mParameters[0].mType = PARAMETERTYPE_UAV; //frust occu draw cmd
	cullrootsig.mParameters[0].mResCounts = 1;
	cullrootsig.mParameters[0].mBindSlot = 0;
	cullrootsig.mParameters[1].mType = PARAMETERTYPE_UAV; // frust norml draw cmd
	cullrootsig.mParameters[1].mResCounts = 1;
	cullrootsig.mParameters[1].mBindSlot = 1;
	cullrootsig.mParameters[2].mType = PARAMETERTYPE_CBV;
	cullrootsig.mParameters[2].mResCounts = 1;
	cullrootsig.mParameters[2].mBindSlot = 0;
	cullrootsig.mParameters[2].mResource = &cameraBuffer;
	cullrootsig.mParameters[3].mType = PARAMETERTYPE_SRV; // all indirect command buffer
	cullrootsig.mParameters[3].mResCounts = 1;
	cullrootsig.mParameters[3].mBindSlot = 0;
	cullrootsig.mParameters[3].mResource = &indirectBuffer;
	cullrootsig.mParameters[4].mType = PARAMETERTYPE_SRV; // instaned data
	cullrootsig.mParameters[4].mResCounts = 1;
	cullrootsig.mParameters[4].mBindSlot = 1;
	cullrootsig.mParameters[4].mResource = &instanceStructBuffer;
	cullrootsig.initialize(render.mDevice);

	ShaderSet cullshader;
	cullshader.shaders[CS].load("Shaders/FrsutumCullingBoxDrawCmd.hlsl", "CSMain", CS);
	cullPipeline.createComputePipeline(render.mDevice, cullrootsig, cullshader);


	occucomdrootsig.mParameters.resize(3);
	occucomdrootsig.mParameters[0].mType = PARAMETERTYPE_UAV;
	occucomdrootsig.mParameters[0].mResCounts = 1;
	occucomdrootsig.mParameters[0].mBindSlot = 0;
	occucomdrootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	occucomdrootsig.mParameters[1].mResCounts = 1;
	occucomdrootsig.mParameters[1].mBindSlot = 0;
	occucomdrootsig.mParameters[2].mType = PARAMETERTYPE_SRV;
	occucomdrootsig.mParameters[2].mResCounts = 1;
	occucomdrootsig.mParameters[2].mBindSlot = 1;
	occucomdrootsig.initialize(render.mDevice);


	ShaderSet occCmdShader;
	occCmdShader.shaders[CS].load("Shaders/OcculusionCulling.hlsl", "CSMain", CS);


	occCmdPipeline.createComputePipeline(render.mDevice, occucomdrootsig, occCmdShader);

	objnum = xnum*ynum*znum;
	occqueheap.initialize(render.mDevice, D3D12_QUERY_HEAP_TYPE_OCCLUSION, objnum);
	clearbuf.resize(objnum,0);





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
	for (int i = 0; i < mesh->mNumVertices; i++) // find min, max
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
	cout << mesh->mNumFaces << endl << endl << endl;
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		indexdata[i * 3] = mesh->mFaces[i].mIndices[0];
		cout << indexdata[i * 3] << endl;

		indexdata[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		cout << indexdata[i * 3+1] << endl;
		indexdata[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
		cout << indexdata[i * 3+2] << endl;
	}

	if (mesh->HasTangentsAndBitangents())
		cout << "Has tangent" << endl;





	indstanceConstBuffer.createConstantBuffer(render.mDevice, sizeof(InstanceData)*objnum); // not register in desricptor heap, so can have larget size const buffer
	instanceStructBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstanceData), objnum, STRUCTERED_BUFFER_TYPE_READ); // use for culling, start to thinking that with dynamic indexing. do we really need to use const buffer for instant data?

	indirectBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ);
	cullIndirectBuffer[0].createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, true);
	cullIndirectBuffer[1].createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, true);
	cullIndirectBuffer[2].createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectCommand), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, true);

	occuIndirectBuffer[0].createStructeredBuffer(render.mDevice, srvheap, sizeof(OccuIndirCommad), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, true);
	occuIndirectBuffer[1].createStructeredBuffer(render.mDevice, srvheap, sizeof(OccuIndirCommad), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, true);
	occuIndirectBuffer[2].createStructeredBuffer(render.mDevice, srvheap, sizeof(OccuIndirCommad), objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, true);

	occResult[0].createStructeredBuffer(render.mDevice, srvheap, 4, objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, false);
	occResult[1].createStructeredBuffer(render.mDevice, srvheap, 4, objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, false);
	occResult[2].createStructeredBuffer(render.mDevice, srvheap, 4, objnum, STRUCTERED_BUFFER_TYPE_READ_WRITE, false);


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

	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceBarrier(vertexBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(normalBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.resourceBarrier(indirectBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(instanceStructBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);


	cmdlist.updateBufferData(vertexBuffer, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(normalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indirectBuffer, commandInf.data(), sizeof(IndirectCommand)*commandInf.size());

	cmdlist.updateBufferData(instanceStructBuffer, instanceInf.data(), objnum * sizeof(InstanceData));


	cmdlist.resourceBarrier(instanceStructBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceBarrier(indirectBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceBarrier(vertexBuffer.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(normalBuffer.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	cmdlist.resourceBarrier(cullIndirectBuffer[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceBarrier(cullIndirectBuffer[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceBarrier(cullIndirectBuffer[2], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceBarrier(occuIndirectBuffer[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdlist.resourceBarrier(occuIndirectBuffer[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdlist.resourceBarrier(occuIndirectBuffer[2], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	cmdlist.resourceBarrier(occResult[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PREDICATION);
	cmdlist.resourceBarrier(occResult[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PREDICATION);
	cmdlist.resourceBarrier(occResult[2], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PREDICATION);




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
	occCmdPipeline.release();
	occucomdrootsig.realease();

	occurootsig.realease();

	occResult[0].release();
	occResult[1].release();
	occResult[2].release();

	occPipeline.release();
	occCmdSig.release();
	occuIndirectBuffer[2].release();
	occuIndirectBuffer[1].release();
	occuIndirectBuffer[0].release();

	occqueheap.release();

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
	camera2.updateViewProj();
	cameraBuffer.updateBufferfromCpu(camera.getMatrix(), sizeof(ViewProjection));
	cameraBuffer2.updateBufferfromCpu(camera2.getMatrix(), sizeof(ViewProjection));
	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(cullPipeline);
	cmdlist.bindDescriptorHeaps(&srvheap);
	// reset the command number and start culling
	cmdlist.resourceBarrier(cullIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST); //reset count
	cmdlist.resourceBarrier(cullIndirectBuffer[(frameIndex+1)%3], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(occuIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(occResult[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.updateBufferData(occResult[0], clearbuf.data(), sizeof(int)*objnum);
	cmdlist.setCounterforStructeredBuffer(occuIndirectBuffer[frameIndex], 0);
	cmdlist.setCounterforStructeredBuffer(cullIndirectBuffer[frameIndex], 0);
	cmdlist.setCounterforStructeredBuffer(cullIndirectBuffer[(frameIndex+1)%3], 0);
	cmdlist.resourceBarrier(occResult[0], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.resourceBarrier(cullIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.resourceBarrier(cullIndirectBuffer[(frameIndex+1)%3], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.resourceBarrier(occuIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);


	// frustum culling
	cmdlist.bindComputeRootSigature(cullrootsig);
	cmdlist.bindComputeResource(0, occuIndirectBuffer[frameIndex]);
	cmdlist.bindComputeResource(1, cullIndirectBuffer[frameIndex]);
	cmdlist.dispatch((objnum + 1023) / 1024, 1, 1);
	cmdlist.resourceBarrier(cullIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceBarrier(occuIndirectBuffer[frameIndex], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);


	






	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.renderTargetBarrier(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex]);
	// before clear render target, start to query the occulusion

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (useocculusion)
	{
		//  start to query occulusion
		cmdlist.bindPipeline(occPipeline);
		cmdlist.bindGraphicsRootSigature(occurootsig);
		cmdlist.bindGraphicsResource(0, cameraBuffer);
		cmdlist.executeIndirect(occCmdSig, objnum, occuIndirectBuffer[frameIndex], 0, occuIndirectBuffer[frameIndex], occuIndirectBuffer[frameIndex].mBufferSize - sizeof(UINT));
		cmdlist.resourceBarrier(occResult[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


		cmdlist.bindPipeline(occCmdPipeline);
		cmdlist.bindComputeRootSigature(occucomdrootsig);
		cmdlist.bindComputeResource(0, cullIndirectBuffer[(frameIndex + 1) % 3]);
		cmdlist.bindComputeResource(1, cullIndirectBuffer[frameIndex]);
		cmdlist.bindComputeResource(2, occResult[0]);
		cmdlist.dispatch((objnum + 1023) / 1024, 1, 1);
		cmdlist.resourceBarrier(cullIndirectBuffer[(frameIndex + 1) % 3], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	else
	{
		cmdlist.resourceBarrier(occResult[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cmdlist.resourceBarrier(cullIndirectBuffer[(frameIndex + 1) % 3], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	// use result to cull unecessary command

	cmdlist.clearDepthStencil(render.mSwapChainRenderTarget[frameIndex]);
	cmdlist.bindIndexBuffer(indexBuffer);
	cmdlist.bindVertexBuffers(vertexBuffer, normalBuffer);



	cmdlist.bindPipeline(pipeline);
	cmdlist.bindGraphicsRootSigature(rootsig);

	//	cmdlist.executeIndirect(cmdSig, objnum, indirectBuffer, 0);

	// execute cmd with predict query buffer
	//if (useocculusion)
	//	cmdlist.bindGraphicsResource(0, cameraBuffer);
	//else
	//	cmdlist.bindGraphicsResource(0, cameraBuffer2);
//	cmdlist.setPrediction(occResult[0], 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
	
		
	if (useocculusion)
		cmdlist.executeIndirect(cmdSig, objnum, cullIndirectBuffer[(frameIndex+1)%3], 0, cullIndirectBuffer[(frameIndex + 1) % 3], cullIndirectBuffer[(frameIndex + 1) % 3].mBufferSize - sizeof(UINT));
	else
		cmdlist.executeIndirect(cmdSig, objnum, cullIndirectBuffer[frameIndex], 0, cullIndirectBuffer[frameIndex], cullIndirectBuffer[frameIndex].mBufferSize - sizeof(UINT));
//	cmdlist.unsetPrediction(D3D12_PREDICATION_OP_EQUAL_ZERO);

	





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
	{
	
			camera.addAngle(curxoffet / 5.0, curyoffet / 5.0);
	
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

			camera.addZoom(1);

	}
	if (yoffset == -1)
	{

			camera.addZoom(-1);

	}
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{


	if (key == GLFW_KEY_1)
		useocculusion = true;
	if (key == GLFW_KEY_2)
		useocculusion = false;
}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "OcclusionQuery");
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