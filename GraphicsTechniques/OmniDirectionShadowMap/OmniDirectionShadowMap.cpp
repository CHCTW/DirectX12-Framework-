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

#include "PointLight.h"
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


ObjectData Spheres;

const UINT rowcount = 10;
const UINT collomcount = 5;
UINT spherecount = rowcount*collomcount;

float radious = 5;
float heightgap = 2;
float rotationoffset = 0.0;
float radian;
float rotationspeed = 0.0005f;


bool lightmove;
ObjectData Buddha;
ObjectData Ground;
PointLight light;



ViewPort shadowViewport;
Scissor shadowScissor;
DescriptorHeap dsvheap;
ShaderSet shadowshaderset;
Pipeline shadowPipeline;
RootSignature shadowRootsig;
DescriptorHeap samplerheap;
FLOAT bordercolor[4] = { 1.0,1.0,1.0,1.0 };
Sampler sampler(D3D12_FILTER_MIN_MAG_MIP_POINT,  // due to pass too many parameters, should use inline to prevent a lot of copy
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	bordercolor);

Texture cubeShadowMap;
vector<Texture> depthBuffer;
const UINT cubeWidth = 256;
const UINT cubeHeight = 256;


void initializeRender()
{



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




	samplerheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	dsvheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	
	//shadowMap.createRenderTargets(render.mDevice, cubeWidth, cubeHeight, shadowformat, dsvheap, srvheap);

	shadowViewport.setup(0.0f, 0.0f, (float)cubeWidth, (float)cubeHeight);
	shadowScissor.setup(0.0f, (float)cubeWidth, 0.0f, (float)cubeHeight);

	RenderTargetFormat shadowformat(0, nullptr, true, true, DXGI_FORMAT_R16_TYPELESS);
	cubeShadowMap.CreateTexture(render,srvheap, shadowformat.mDepthStencilFormat, cubeWidth, cubeHeight,1,1,TEXTURE_SRV_TYPE_CUBE,TEXTURE_USAGE_SRV_DSV);


	sampler.createSampler(samplerheap);

	rootsig.mParameters.resize(5);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;
	rootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 0;
	rootsig.mParameters[1].mResource = &Spheres.mStructeredBuffer;
	rootsig.mParameters[2].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[2].mResCounts = 1;
	rootsig.mParameters[2].mBindSlot = 1;
	rootsig.mParameters[2].mResource = &lightBuffer;
	rootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootsig.mParameters[3].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[3].mResCounts = 1;
	rootsig.mParameters[3].mBindSlot = 1;
	rootsig.mParameters[3].mResource = &cubeShadowMap; // since threre is not even a depth texture create before create render target, need to create rendertarget for shadow map first
	rootsig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootsig.mParameters[4].mType = PARAMETERTYPE_SAMPLER;
	rootsig.mParameters[4].mResCounts = 1;
	rootsig.mParameters[4].mBindSlot = 0;
	rootsig.mParameters[4].mSampler = &sampler;
	rootsig.mParameters[4].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/BRDFCubeShadow.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/BRDFCubeShadow.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT), VERTEX_LAYOUT_TYPE_SPLIT_ALL);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.updateViewProj();




	shadowRootsig.mParameters.resize(3);
	shadowRootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	shadowRootsig.mParameters[0].mResCounts = 1;
	shadowRootsig.mParameters[0].mBindSlot = 0;
	shadowRootsig.mParameters[0].mResource = &lightBuffer;
	shadowRootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	shadowRootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	shadowRootsig.mParameters[1].mResCounts = 1;
	shadowRootsig.mParameters[1].mBindSlot = 0;
	shadowRootsig.mParameters[1].mResource = &Spheres.mStructeredBuffer;
	shadowRootsig.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	shadowRootsig.mParameters[2].mType = PARAMETERTYPE_ROOTCONSTANT;
	shadowRootsig.mParameters[2].mResCounts = 1;
	shadowRootsig.mParameters[2].mBindSlot = 1;
	shadowRootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	shadowRootsig.initialize(render.mDevice);


	shadowshaderset.shaders[VS].load("Shaders/CubeShadowMap.hlsl", "VSMain", VS);
	shadowshaderset.shaders[PS].load("Shaders/CubeShadowMap.hlsl", "PSMain", PS);
	shadowshaderset.shaders[GS].load("Shaders/CubeShadowMap.hlsl", "GSMain", GS);
//	RenderTargetFormat shadowformat(0, nullptr, true,true, DXGI_FORMAT_R16_TYPELESS);
	shadowPipeline.createGraphicsPipeline(render.mDevice, shadowRootsig, shadowshaderset, shadowformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT), VERTEX_LAYOUT_TYPE_SPLIT_ALL);

}

void loadAsset()
{


	camera.mZoom = 15;

	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();

	lightBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(PointLightData));
	lightBuffer.maptoCpu();


	light.setColor(1, 1, 1);
	light.setIntensity(50);
	light.setRadius(40);
	light.mZoom = 0;

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
	Buddha.mPosition[0].setScale(1, 1, 1);
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
	Ground.mPosition[0].setScale(10, 10, 10);
	Ground.mPosition[0].setPosition(0, 0, 0);
	Ground.mPosition[0].CacNewTransform();
	Ground.mBufferData[0].mMatrices = Ground.mPosition[0].getMatrices();
	Ground.mBufferData[0].mMaterial.mAlbedo = glm::vec3(0.55, 0.55, 0.55);
	Ground.mBufferData[0].mMaterial.mRoughness = 0.8;
	Ground.mBufferData[0].mMaterial.mMetallic = 0.8;



	// load sphere data

	import.ReadFile("Assets/sphere.obj", aiProcessPreset_TargetRealtime_Quality);
	scene = import.GetScene();
	mesh = scene->mMeshes[0];

	UINT verticnum = mesh->mNumVertices;

	if (!Spheres.mVertexBufferData.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float)))
	{
		cout << "Fail for create" << endl;
	}
	Spheres.mNormalBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));


	Spheres.mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * 3 * mesh->mNumFaces);

	std::vector<unsigned int> indexdata;
	indexdata.resize(mesh->mNumFaces * 3);
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		indexdata[i * 3] = mesh->mFaces[i].mIndices[0];
		indexdata[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		indexdata[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}

	// initialize spheres position data
	Spheres.mNum = spherecount;
	Spheres.indexCount = mesh->mNumFaces * 3;
	Spheres.mBufferData.resize(Spheres.mNum);
	Spheres.mPosition.resize(Spheres.mNum);

	Spheres.mStructeredBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Spheres.mNum, STRUCTERED_BUFFER_TYPE_READ);

	radian = 2 * 3.14159f / rowcount;
	float rough = 1.0f / (rowcount - 1);
	float metalic = 1.0f / (collomcount - 1);
	for (int i = 0; i < Spheres.mNum; ++i)
	{
		int height = i / rowcount;
		int rowpos = i % rowcount;
		Spheres.mPosition[i].setPosition(radious*cos(i*radian), heightgap*height, radious*sin(i*radian));
		//Spheres.mPosition[i].setAngle(-90, 0, 0);
		Spheres.mPosition[i].CacNewTransform();
		Spheres.mBufferData[i].mMatrices = Spheres.mPosition[i].getMatrices();
		Spheres.mBufferData[i].mMaterial.mAlbedo = glm::vec3(1.0, 0.0, 0.0);
		Spheres.mBufferData[i].mMaterial.mRoughness = (rowpos)*rough;
		Spheres.mBufferData[i].mMaterial.mMetallic = height*metalic;

	}




	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(Spheres.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Spheres.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Spheres.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Spheres.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Spheres.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);


	cmdlist.updateBufferData(Spheres.mVertexBufferData, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Spheres.mIndexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Spheres.mNormalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Spheres.mStructeredBuffer, Spheres.mBufferData.data(), Spheres.mNum * sizeof(InstancedInformation));

	cmdlist.updateBufferData(Buddha.mVertexBufferData, buddha->mVertices, buddha->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Buddha.mIndexBuffer, buddhaindexdata.data(), buddha->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Buddha.mNormalBuffer, buddha->mNormals, buddha->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Buddha.mStructeredBuffer, Buddha.mBufferData.data(), Buddha.mNum * sizeof(InstancedInformation));

	cmdlist.updateBufferData(Ground.mVertexBufferData, ground->mVertices, ground->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Ground.mIndexBuffer, groundindexdata.data(), ground->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Ground.mNormalBuffer, ground->mNormals, ground->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Ground.mStructeredBuffer, Ground.mBufferData.data(), Ground.mNum * sizeof(InstancedInformation));

	cmdlist.resourceTransition(Spheres.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Spheres.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Spheres.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Spheres.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceTransition(Buddha.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceTransition(cubeShadowMap, D3D12_RESOURCE_STATE_GENERIC_READ,true);


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
	cubeShadowMap.release();
	samplerheap.release();
	shadowPipeline.release();
	shadowRootsig.realease();
	dsvheap.release();

	Ground.mNormalBuffer.release();
	Ground.mVertexBufferData.release();
	Ground.mStructeredBuffer.release();
	Ground.mIndexBuffer.release();
	lightBuffer.release();
	Buddha.mNormalBuffer.release();
	Buddha.mVertexBufferData.release();
	Buddha.mStructeredBuffer.release();
	Buddha.mIndexBuffer.release();
	Spheres.mStructeredBuffer.release();
	import.FreeScene();
	cameraBuffer.release();
	Spheres.mIndexBuffer.release();
	Spheres.mVertexBufferData.release();
	srvheap.release();
	pipeline.release();
	rootsig.realease();
	Spheres.mNormalBuffer.release();
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
	lightBuffer.updateBufferfromCpu(light.getLightData(), sizeof(PointLightData));
	//radian = 2 * 3.14159f / Spheres.mNum;
	rotationoffset += rotationspeed;
	for (int i = 0; i < Spheres.mNum; ++i)
	{
		int height = i / rowcount;
		int rowpos = i % rowcount;
		Spheres.mPosition[i].setPosition(radious*cos(rowpos*radian + rotationoffset), heightgap*height - (collomcount*heightgap) / 2, radious*sin(rowpos*radian + rotationoffset));

		//	Spheres.mPosition[i].setPosition(rowpos*1.5, height, 5);
		//Spheres.mPosition[i].setAngle(0, (rowpos*radian+ rotationoffset)/3.14159*180, 0);
		Spheres.mPosition[i].CacNewTransform();
		Spheres.mBufferData[i].mMatrices = Spheres.mPosition[i].getMatrices();
	}

}
void onrender()
{

	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset();
	cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);

	{
		cmdlist.resourceTransition(Spheres.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
		cmdlist.updateBufferData(Spheres.mStructeredBuffer, Spheres.mBufferData.data(), Spheres.mNum * sizeof(InstancedInformation));
		cmdlist.resourceTransition(Spheres.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	{
	//	cmdlist.depthBufferBarrier(shadowMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE); // start shadow pass
		cmdlist.resourceTransition(cubeShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

		cmdlist.bindPipeline(shadowPipeline);
		cmdlist.bindGraphicsRootSigature(shadowRootsig, true);
		cmdlist.setViewPort(shadowViewport);
		cmdlist.setScissor(shadowScissor);

		cmdlist.bindDepthStencilBufferOnly(cubeShadowMap);
		cmdlist.clearDepthStencil(cubeShadowMap);

//		cmdlist.bindRenderTarget(shadowMap);
//		cmdlist.clearDepthStencil(shadowMap);
		for (int face = 0; face < 6; face++)
		{


			///cmdlist.clearcubeDepthStencil(cubeShadowMap, face, 1);
			cmdlist.bindGraphicsConstant(2, &face);

			cmdlist.bindGraphicsResource(1, Spheres.mStructeredBuffer);
			cmdlist.bindIndexBuffer(Spheres.mIndexBuffer);
			cmdlist.bindVertexBuffers(Spheres.mVertexBufferData, Spheres.mNormalBuffer);
			cmdlist.drawIndexedInstanced(Spheres.indexCount, Spheres.mNum, 0, 0);

			//cmdlist.bindGraphicsResource(1, Buddha.mStructeredBuffer);
			//cmdlist.bindIndexBuffer(Buddha.mIndexBuffer);
			//cmdlist.bindVertexBuffers(Buddha.mVertexBufferData, Buddha.mNormalBuffer);
			//cmdlist.drawIndexedInstanced(Buddha.indexCount, Buddha.mNum, 0, 0);


			cmdlist.bindGraphicsResource(1, Ground.mStructeredBuffer);
			cmdlist.bindIndexBuffer(Ground.mIndexBuffer);
			cmdlist.bindVertexBuffers(Ground.mVertexBufferData, Ground.mNormalBuffer);
			cmdlist.drawIndexedInstanced(Ground.indexCount, Ground.mNum, 0, 0);

		}

		cmdlist.resourceTransition(cubeShadowMap,D3D12_RESOURCE_STATE_GENERIC_READ,true);
		//cmdlist.depthBufferBarrier(shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
	}



	cmdlist.bindPipeline(pipeline);
	cmdlist.bindGraphicsRootSigature(rootsig, true);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex],depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist.bindGraphicsResource(1, Spheres.mStructeredBuffer);
	cmdlist.bindIndexBuffer(Spheres.mIndexBuffer);
	cmdlist.bindVertexBuffers(Spheres.mVertexBufferData, Spheres.mNormalBuffer);
	cmdlist.drawIndexedInstanced(Spheres.indexCount, Spheres.mNum, 0, 0);

	//cmdlist.bindGraphicsResource(1, Buddha.mStructeredBuffer);
	//cmdlist.bindIndexBuffer(Buddha.mIndexBuffer);
	//cmdlist.bindVertexBuffers(Buddha.mVertexBufferData, Buddha.mNormalBuffer);
	//cmdlist.drawIndexedInstanced(Buddha.indexCount, Buddha.mNum, 0, 0);

	cmdlist.bindGraphicsResource(1, Ground.mStructeredBuffer);
	cmdlist.bindIndexBuffer(Ground.mIndexBuffer);
	cmdlist.bindVertexBuffers(Ground.mVertexBufferData, Ground.mNormalBuffer);
	cmdlist.drawIndexedInstanced(Ground.indexCount, Ground.mNum, 0, 0);
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
		light.setColor(1.0, 0.0, 0.0);
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
	windows.initialize(1600, 900, "OmniDirectionShadowMap");
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