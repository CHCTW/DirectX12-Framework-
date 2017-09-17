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
#include "PointLight.h"
#include <array>
#include <random>
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
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
DescriptorHeap srvheap;
DescriptorHeap rtvheap;
DescriptorHeap dsvheap;
DescriptorHeap samplerheap;

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


UINT spherecount = 200;

float radious = 5;
float heightgap = 1.5;
float rotationoffset = 0.0;
float radian;
float rotationspeed = 0.0005f;


bool lightmove;
ObjectData Buddha;
ObjectData Ground;
SpotLight light;


RenderTarget GBuffer;

ShaderSet defferedshader;
Pipeline defferedpipeline;
RootSignature defferedrootsig;

ShaderSet quadshader;
Pipeline quadpipeline;
RootSignature quadrootsig;
Sampler sampler;

ShaderSet localshader;
ViewPort debugviewport;
std::vector<PointLightData> lightlist;
static unsigned int lightcount = 500;
Buffer lightDataBuffer;
Pipeline localLightpPipeline;

std::default_random_engine generator;
std::uniform_real_distribution<float> distributionXZ(-50.0, 50.0);
std::uniform_real_distribution<float> distributionY(-3.0,0.0);
std::uniform_real_distribution<float> distributionmaterial(0.3, 0.8);
std::uniform_real_distribution<float> distributionscale(1.0, 5.0);


std::uniform_real_distribution<float> distributionradius(5.0, 10.0);
std::uniform_real_distribution<float> distributionlightY(0.0, 2.0);
std::array<double, 3> intervals{ 0.1, 0.5, 0.9 };
std::array<float, 3> weights{ 10.0, 0.0, 10.0 };
std::piecewise_linear_distribution<double>
distributionlightcolor(intervals.begin(), intervals.end(), weights.begin());

//std::uniform_real_distribution<float> distributionlightcolor(0.3, 1.5);
std::uniform_real_distribution<float> distributionmove(0.0, 0.1);

void initializeRender()
{


	//Camera cam;



	render.initialize();
	RenderTargetFormat retformat;
	render.createSwapChain(windows, swapChainCount, retformat);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render.mDevice);
	fence.fenceValue = 1;
	fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);


	rtvheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	samplerheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	srvheap.ininitialize(render.mDevice, 1);

	rootsig.mParameters.resize(2);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;
	rootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 0;
	rootsig.mParameters[1].mResource = &Spheres.mStructeredBuffer;
	rootsig.initialize(render.mDevice);

	quadrootsig.mParameters.resize(2);
	quadrootsig.mParameters[0].mType = PARAMETERTYPE_SRV;
	quadrootsig.mParameters[0].mResCounts = 1;
	quadrootsig.mParameters[0].mBindSlot = 0;
	//rootsig.mParameters[0].mResource = &Spheres.mStructeredBuffer;
	quadrootsig.mParameters[1].mType = PARAMETERTYPE_SAMPLER;
	quadrootsig.mParameters[1].mResCounts = 1;
	quadrootsig.mParameters[1].mBindSlot = 0;
	quadrootsig.mParameters[1].mSampler= &sampler;
	quadrootsig.initialize(render.mDevice);



	DXGI_FORMAT formats[2];
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT; // normal + metal
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT; // color + roughness
	RenderTargetFormat gBufferFormat(2, formats, true);
	GBuffer.createRenderTargets(render.mDevice, windows.mWidth, windows.mHeight, gBufferFormat, rtvheap, dsvheap, srvheap);

	shaderset.shaders[VS].load("Shaders/GBuffer.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/GBuffer.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, gBufferFormat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT), VERTEX_LAYOUT_TYPE_SPLIT_ALL);


	quadshader.shaders[VS].load("Shaders/FullScreenQuad.hlsl", "VSMain", VS);
	quadshader.shaders[PS].load("Shaders/FullScreenQuad.hlsl", "PSMain", PS);

	quadpipeline.createGraphicsPipeline(render.mDevice, quadrootsig, quadshader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_NONE_SPLIT);

	defferedrootsig.mParameters.resize(6);
	defferedrootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	defferedrootsig.mParameters[0].mResCounts = 1;
	defferedrootsig.mParameters[0].mBindSlot = 0;
	defferedrootsig.mParameters[0].mResource = &cameraBuffer;
	defferedrootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[1].mResCounts = 1;
	defferedrootsig.mParameters[1].mBindSlot = 0;
	defferedrootsig.mParameters[1].mResource = &GBuffer.mRenderBuffers[0];
	defferedrootsig.mParameters[2].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[2].mResCounts = 1;
	defferedrootsig.mParameters[2].mBindSlot = 1;
	defferedrootsig.mParameters[2].mResource = &GBuffer.mRenderBuffers[1];
	defferedrootsig.mParameters[3].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[3].mResCounts = 1;
	defferedrootsig.mParameters[3].mBindSlot = 2;
	defferedrootsig.mParameters[3].mResource = &GBuffer.mDepthBuffer[0];
	defferedrootsig.mParameters[4].mType = PARAMETERTYPE_SAMPLER;
	defferedrootsig.mParameters[4].mResCounts = 1;
	defferedrootsig.mParameters[4].mBindSlot = 0;
	defferedrootsig.mParameters[4].mSampler = &sampler;
	defferedrootsig.mParameters[5].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[5].mResCounts = 1;
	defferedrootsig.mParameters[5].mBindSlot = 3;
	defferedrootsig.mParameters[5].mResource = &lightDataBuffer;
	defferedrootsig.mParameters[5].mVisibility = D3D12_SHADER_VISIBILITY_ALL;




	defferedrootsig.initialize(render.mDevice);


	defferedshader.shaders[VS].load("Shaders/DefferedShadingAmbient.hlsl", "VSMain", VS);
	defferedshader.shaders[PS].load("Shaders/DefferedShadingAmbient.hlsl", "PSMain", PS);

	defferedpipeline.createGraphicsPipeline(render.mDevice, defferedrootsig, defferedshader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(false), RasterizerState::RasterizerState());



	localshader.shaders[VS].load("Shaders/DefferedShadingLocalLight.hlsl", "VSMain", VS);
	localshader.shaders[PS].load("Shaders/DefferedShadingLocalLight.hlsl", "PSMain", PS);

	localLightpPipeline.createGraphicsPipeline(render.mDevice, defferedrootsig, localshader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(true), RasterizerState::RasterizerState(D3D12_CULL_MODE_BACK));



	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.updateViewProj();


}

void loadAsset()
{


	sampler.createSampler(samplerheap);

	camera.mZoom = 15;

	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();



	light.setRadius(200);

	buddhaimport.ReadFile("Assets/buddha.obj", aiProcessPreset_TargetRealtime_Fast);

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
	Buddha.mPosition[0].setAngle(-90, 0, 0);
	Buddha.mPosition[0].setScale(5, 5, 5);
	//Buddha.mPosition[0].setPosition(0, 0, 5);
	Buddha.mPosition[0].CacNewTransform();
	Buddha.mBufferData[0].mMatrices = Buddha.mPosition[0].getMatrices();
	Buddha.mBufferData[0].mMaterial.mAlbedo = glm::vec3(1.00, 0.71, 0.29);
	Buddha.mBufferData[0].mMaterial.mRoughness = 0.4;
	Buddha.mBufferData[0].mMaterial.mMetallic = 1.0;


	groundimport.ReadFile("Assets/cube.obj", aiProcessPreset_TargetRealtime_Fast);
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
	Ground.mPosition[0].setScale(50, 1, 50);
	Ground.mPosition[0].setPosition(0, -5, 0);
	Ground.mPosition[0].CacNewTransform();
	Ground.mBufferData[0].mMatrices = Ground.mPosition[0].getMatrices();
	Ground.mBufferData[0].mMaterial.mAlbedo = glm::vec3(0.55, 0.55, 0.55);
	Ground.mBufferData[0].mMaterial.mRoughness = 0.8;
	Ground.mBufferData[0].mMaterial.mMetallic = 0.8;





	// load sphere data

	import.ReadFile("Assets/sphere.obj", aiProcessPreset_TargetRealtime_Fast);
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

	for (int i = 0; i < Spheres.mNum; ++i)
	{
		//int height = i / rowcount;
		//int rowpos = i % rowcount;
		Spheres.mPosition[i].setPosition(distributionXZ(generator), distributionY(generator), distributionXZ(generator));
		float s = distributionscale(generator);
		Spheres.mPosition[i].setScale(s, s, s);
		//Spheres.mPosition[i].setAngle(-90, 0, 0);
		Spheres.mPosition[i].CacNewTransform();
		Spheres.mBufferData[i].mMatrices = Spheres.mPosition[i].getMatrices();
		Spheres.mBufferData[i].mMaterial.mAlbedo = glm::vec3(distributionmaterial(generator), distributionmaterial(generator), distributionmaterial(generator));
	//	Spheres.mBufferData[i].mMaterial.mAlbedo = glm::vec3(0.6);
//		Spheres.mBufferData[i].mMaterial.mAlbedo = glm::vec3(distributionlightcolor(generator), distributionlightcolor(generator), distributionlightcolor(generator));
		Spheres.mBufferData[i].mMaterial.mRoughness = distributionmaterial(generator);
		Spheres.mBufferData[i].mMaterial.mMetallic = distributionmaterial(generator);

	//	Spheres.mBufferData[i].mMaterial.mMetallic = 0.0f;

	}




	lightDataBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(PointLightData), lightcount, STRUCTERED_BUFFER_TYPE_READ);

	lightlist.resize(lightcount);


	PointLight light;
	for (int i = 0; i < lightcount; ++i)
	{
		light.setRadius(distributionradius(generator));
		light.setColor(distributionlightcolor(generator), distributionlightcolor(generator), distributionlightcolor(generator));
		light.setPosition(distributionXZ(generator), distributionlightY(generator),distributionXZ(generator));
		lightlist[i] = *light.getLightData();
	}



	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceBarrier(Spheres.mVertexBufferData.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Spheres.mNormalBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Spheres.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Spheres.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Buddha.mVertexBufferData.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Buddha.mNormalBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.resourceBarrier(Ground.mVertexBufferData.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Ground.mNormalBuffer.mResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceBarrier(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.resourceBarrier(lightDataBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);



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

	cmdlist.updateBufferData(lightDataBuffer, lightlist.data(), lightlist.size() * sizeof(PointLightData));


	cmdlist.resourceBarrier(Spheres.mVertexBufferData.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(Spheres.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(Spheres.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceBarrier(Spheres.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceBarrier(Buddha.mVertexBufferData.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceBarrier(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceBarrier(Ground.mVertexBufferData.mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceBarrier(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceBarrier(Ground.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceBarrier(lightDataBuffer, D3D12_RESOURCE_STATE_COPY_DEST,D3D12_RESOURCE_STATE_GENERIC_READ);


	cmdlist.renderTargetBarrier(GBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.depthBufferBarrier(GBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);


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

	localLightpPipeline.release();
	lightDataBuffer.release();

	defferedrootsig.realease();
	defferedpipeline.release();
	quadpipeline.release();
	quadrootsig.realease();
	samplerheap.release();

	GBuffer.release();
	rtvheap.release();
	dsvheap.release();

	Ground.mNormalBuffer.release();
	Ground.mVertexBufferData.release();
	Ground.mStructeredBuffer.release();
	Ground.mIndexBuffer.release();
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



	//radian = 2 * 3.14159f / Spheres.mNum;
	rotationoffset += rotationspeed;
	//for (int i = 0; i < Spheres.mNum; ++i)
	//{
	//	int height = i / rowcount;
	//	int rowpos = i % rowcount;
	//	Spheres.mPosition[i].setPosition(radious*cos(rowpos*radian + rotationoffset), heightgap*height - (collomcount*heightgap) / 2, radious*sin(rowpos*radian + rotationoffset));

	//	//	Spheres.mPosition[i].setPosition(rowpos*1.5, height, 5);
	//	//Spheres.mPosition[i].setAngle(0, (rowpos*radian+ rotationoffset)/3.14159*180, 0);
	//	Spheres.mPosition[i].CacNewTransform();
	//	Spheres.mBufferData[i].mMatrices = Spheres.mPosition[i].getMatrices();
	//}


	for (int i = 0; i < lightcount; ++i)
	{

		lightlist[i].mPosition.x+= distributionmove(generator);
		

		if (lightlist[i].mPosition.x >= 60)
		{
			lightlist[i].mPosition.x = -60;
			lightlist[i].mColor.r = distributionmaterial(generator);
			lightlist[i].mColor.g = distributionmaterial(generator);
			lightlist[i].mColor.b = distributionmaterial(generator);
		}
	}

}
void onrender()
{

	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(pipeline);

	cmdlist.resourceBarrier(Spheres.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);


	cmdlist.resourceBarrier(lightDataBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.updateBufferData(Spheres.mStructeredBuffer, Spheres.mBufferData.data(), Spheres.mNum * sizeof(InstancedInformation));
	cmdlist.updateBufferData(lightDataBuffer, lightlist.data(), lightlist.size() * sizeof(PointLightData));

	cmdlist.resourceBarrier(lightDataBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceBarrier(Spheres.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	{ // GBuffer - Pass
		cmdlist.setViewPort(viewport);
		cmdlist.setScissor(scissor);
		cmdlist.bindGraphicsRootSigature(rootsig);


		//cmdlist.renderTargetBarrier(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		cmdlist.renderTargetBarrier(GBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdlist.depthBufferBarrier(GBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		cmdlist.bindRenderTarget(GBuffer);
		cmdlist.clearRenderTarget(GBuffer);
		cmdlist.clearDepthStencil(GBuffer);


		cmdlist.bindIndexBuffer(Spheres.mIndexBuffer);
		cmdlist.bindVertexBuffers(Spheres.mVertexBufferData, Spheres.mNormalBuffer);
		cmdlist.drawIndexedInstanced(Spheres.indexCount, Spheres.mNum, 0, 0);

		cmdlist.bindGraphicsResource(1, Buddha.mStructeredBuffer);
		cmdlist.bindIndexBuffer(Buddha.mIndexBuffer);
		cmdlist.bindVertexBuffers(Buddha.mVertexBufferData, Buddha.mNormalBuffer);
		cmdlist.drawIndexedInstanced(Buddha.indexCount, Buddha.mNum, 0, 0);


		cmdlist.bindGraphicsResource(1, Ground.mStructeredBuffer);
		cmdlist.bindIndexBuffer(Ground.mIndexBuffer);
		cmdlist.bindVertexBuffers(Ground.mVertexBufferData, Ground.mNormalBuffer);
		cmdlist.drawIndexedInstanced(Ground.indexCount, Ground.mNum, 0, 0);


		cmdlist.renderTargetBarrier(GBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdlist.depthBufferBarrier(GBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);

	}
	cmdlist.renderTargetBarrier(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	{ //deffered pass
		cmdlist.bindGraphicsRootSigature(defferedrootsig);
		cmdlist.bindPipeline(defferedpipeline);
		cmdlist.drawInstance(3, 1, 0, 0);

		cmdlist.bindPipeline(localLightpPipeline);
		cmdlist.bindIndexBuffer(Spheres.mIndexBuffer);
		cmdlist.bindVertexBuffers(Spheres.mVertexBufferData, Spheres.mNormalBuffer);
		cmdlist.drawIndexedInstanced(Spheres.indexCount, lightcount, 0, 0);

	}
	{ // debug pass
		cmdlist.bindGraphicsRootSigature(quadrootsig);
		cmdlist.bindPipeline(quadpipeline);

		debugviewport.setup(0.0f, 0.0f, (float)windows.mWidth / 4, (float)windows.mHeight / 4);
		cmdlist.setViewPort(debugviewport);
		cmdlist.bindGraphicsResource(0, GBuffer.mRenderBuffers[0]);
		cmdlist.drawInstance(3, 1, 0, 0);
		debugviewport.setup(0.0f, (float)windows.mHeight / 4, (float)windows.mWidth / 4, (float)windows.mHeight / 4);
		cmdlist.setViewPort(debugviewport);
		cmdlist.bindGraphicsResource(0, GBuffer.mRenderBuffers[1]);
		cmdlist.drawInstance(3, 1, 0, 0);
		debugviewport.setup(0.0f, 2*(float)windows.mHeight / 4, (float)windows.mWidth / 4, (float)windows.mHeight / 4);
		cmdlist.setViewPort(debugviewport);
		cmdlist.bindGraphicsResource(0, GBuffer.mDepthBuffer[0]);
		cmdlist.drawInstance(3, 1, 0, 0);


		
	}
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
	windows.initialize(1600, 900, "Deffered Shading");
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