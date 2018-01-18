/*
Using the method from this tutorial:
https://learnopengl.com/#!PBR/IBL/Diffuse-irradiance

*/


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
#include "Image.h"
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


ObjectData Bunnys;


const UINT rowcount = 10;
const UINT collomcount = 5;
UINT spherecount = rowcount*collomcount;

float radious = 10;
float heightgap = 3;
float rotationoffset = 0.0;
float radian;
float rotationspeed = 0.0005f;


bool lightmove;
ObjectData Buddha;
ObjectData Cube;
SpotLight light;


vector<Texture> GBuffer;

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
static unsigned int lightcount = 20;
Buffer lightDataBuffer;
Pipeline localLightpPipeline;

std::default_random_engine generator(1);
std::uniform_real_distribution<float> distributionXZ(-55.0, 55.0);
std::uniform_real_distribution<float> distributionY(-3.0, 0.0);
std::uniform_real_distribution<float> distributionmaterial(0.0, 1.0);
std::uniform_real_distribution<float> distributionscale(1.0, 5.0);


std::uniform_real_distribution<float> distributionradius(15.0, 20.0);
std::uniform_real_distribution<float> distributionlightY(1.0, 2.0);
std::array<double, 3> intervals{ 0.1, 10, 20.0 };
std::array<float, 3> weights{ 0.0, 50.0, 100.0 };
std::piecewise_linear_distribution<double>
distributionlightcolor(intervals.begin(), intervals.end(), weights.begin());

//std::uniform_real_distribution<float> distributionlightcolor(0.3, 1.5);
std::uniform_real_distribution<float> distributionmove(0.0, 0.1);

Texture skyBox;
ShaderSet skyBoxshader;
Pipeline skyBoxPipeline;
RootSignature skyboxRootsig;
float windowSize[2];

Texture IrradianceMap;
UINT IrrWidth = 32;
UINT IrrHeight = 32;
Texture SpecularMap;
UINT SpecWidth = 512;
UINT SpecHeight = 512;
UINT specularlod = 5;


Texture BRDFIntergrateMap;
UINT BRDFIntWidth = 512;
UINT BRDFIntHeight = 512;
void initializeRender()
{


	//Camera cam;



	render.initialize();
	RenderTargetFormat retformat;
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);

	fence.initialize(render.mDevice);
	fence.fenceValue = 1;
	fenceEvet = CreateEvent(NULL, FALSE, FALSE, NULL);


	windowSize[0] = (float)windows.mWidth;
	windowSize[1] = (float)windows.mHeight;


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
	rootsig.mParameters[1].mResource = &Bunnys.mStructeredBuffer;
	rootsig.initialize(render.mDevice);

	quadrootsig.mParameters.resize(2);
	quadrootsig.mParameters[0].mType = PARAMETERTYPE_SRV;
	quadrootsig.mParameters[0].mResCounts = 1;
	quadrootsig.mParameters[0].mBindSlot = 0;
	quadrootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	//rootsig.mParameters[0].mResource = &Bunnys.mStructeredBuffer;
	quadrootsig.mParameters[1].mType = PARAMETERTYPE_SAMPLER;
	quadrootsig.mParameters[1].mResCounts = 1;
	quadrootsig.mParameters[1].mBindSlot = 0;
	quadrootsig.mParameters[1].mSampler = &sampler;
	quadrootsig.initialize(render.mDevice);



	DXGI_FORMAT formats[2];
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT; // normal + metal
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT; // color + roughness
	RenderTargetFormat gBufferFormat(2, formats, true);
	//GBuffer.createRenderTargets(render.mDevice, windows.mWidth, windows.mHeight, gBufferFormat, rtvheap, dsvheap, srvheap);

	GBuffer.resize(3);
	GBuffer[0].CreateTexture(render, srvheap, gBufferFormat.mRenderTargetFormat[0], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);
	GBuffer[1].CreateTexture(render, srvheap, gBufferFormat.mRenderTargetFormat[1], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);
	GBuffer[2].CreateTexture(render, srvheap, gBufferFormat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_DSV);





	shaderset.shaders[VS].load("Shaders/GBuffer.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/GBuffer.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, gBufferFormat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT), VERTEX_LAYOUT_TYPE_SPLIT_ALL);


	quadshader.shaders[VS].load("Shaders/FullScreenQuad.hlsl", "VSMain", VS);
	quadshader.shaders[PS].load("Shaders/FullScreenQuad.hlsl", "PSMain", PS);

	quadpipeline.createGraphicsPipeline(render.mDevice, quadrootsig, quadshader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_NONE_SPLIT);

	defferedrootsig.mParameters.resize(9);
	defferedrootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	defferedrootsig.mParameters[0].mResCounts = 1;
	defferedrootsig.mParameters[0].mBindSlot = 0;
	defferedrootsig.mParameters[0].mResource = &cameraBuffer;
	defferedrootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[1].mResCounts = 1;
	defferedrootsig.mParameters[1].mBindSlot = 0;
	defferedrootsig.mParameters[1].mResource = &GBuffer[0];
	defferedrootsig.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	defferedrootsig.mParameters[2].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[2].mResCounts = 1;
	defferedrootsig.mParameters[2].mBindSlot = 1;
	defferedrootsig.mParameters[2].mResource = &GBuffer[1];
	defferedrootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	defferedrootsig.mParameters[3].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[3].mResCounts = 1;
	defferedrootsig.mParameters[3].mBindSlot = 2;
	defferedrootsig.mParameters[3].mResource = &GBuffer[2];
	defferedrootsig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	defferedrootsig.mParameters[4].mType = PARAMETERTYPE_SAMPLER;
	defferedrootsig.mParameters[4].mResCounts = 1;
	defferedrootsig.mParameters[4].mBindSlot = 0;
	defferedrootsig.mParameters[4].mSampler = &sampler;
	defferedrootsig.mParameters[5].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[5].mResCounts = 1;
	defferedrootsig.mParameters[5].mBindSlot = 3;
	defferedrootsig.mParameters[5].mResource = &lightDataBuffer;
	defferedrootsig.mParameters[5].mVisibility = D3D12_SHADER_VISIBILITY_ALL;

	defferedrootsig.mParameters[6].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[6].mResCounts = 1;
	defferedrootsig.mParameters[6].mBindSlot = 4;
	defferedrootsig.mParameters[6].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	defferedrootsig.mParameters[7].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[7].mResCounts = 1;
	defferedrootsig.mParameters[7].mBindSlot = 5;
	defferedrootsig.mParameters[7].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	defferedrootsig.mParameters[8].mType = PARAMETERTYPE_SRV;
	defferedrootsig.mParameters[8].mResCounts = 1;
	defferedrootsig.mParameters[8].mBindSlot = 6;
	defferedrootsig.mParameters[8].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;




	defferedrootsig.initialize(render.mDevice);


	defferedshader.shaders[VS].load("Shaders/DefferedShadingImagedBasedLigting.hlsl", "VSMain", VS);
	defferedshader.shaders[PS].load("Shaders/DefferedShadingImagedBasedLigting.hlsl", "PSMain", PS);

	defferedpipeline.createGraphicsPipeline(render.mDevice, defferedrootsig, defferedshader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(false), RasterizerState::RasterizerState());



	localshader.shaders[VS].load("Shaders/DefferedShadingLocalLight.hlsl", "VSMain", VS);
	localshader.shaders[PS].load("Shaders/DefferedShadingLocalLight.hlsl", "PSMain", PS);

	localLightpPipeline.createGraphicsPipeline(render.mDevice, defferedrootsig, localshader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(true), RasterizerState::RasterizerState(D3D12_CULL_MODE_BACK));




	skyboxRootsig.mParameters.resize(5);
	skyboxRootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	skyboxRootsig.mParameters[0].mResCounts = 1;
	skyboxRootsig.mParameters[0].mBindSlot = 0;
	skyboxRootsig.mParameters[0].mResource = &cameraBuffer;
	skyboxRootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	skyboxRootsig.mParameters[1].mResCounts = 1;
	skyboxRootsig.mParameters[1].mBindSlot = 0;
	skyboxRootsig.mParameters[1].mResource = &skyBox;
	skyboxRootsig.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	skyboxRootsig.mParameters[2].mType = PARAMETERTYPE_SAMPLER;
	skyboxRootsig.mParameters[2].mResCounts = 1;
	skyboxRootsig.mParameters[2].mBindSlot = 0;
	skyboxRootsig.mParameters[2].mSampler = &sampler;
	skyboxRootsig.mParameters[3].mType = PARAMETERTYPE_SRV;
	skyboxRootsig.mParameters[3].mResCounts = 1;
	skyboxRootsig.mParameters[3].mBindSlot = 1;
	skyboxRootsig.mParameters[3].mResource = &GBuffer[2];
	skyboxRootsig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	skyboxRootsig.mParameters[4].mType = PARAMETERTYPE_ROOTCONSTANT;
	skyboxRootsig.mParameters[4].mResCounts = 2;
	skyboxRootsig.mParameters[4].mBindSlot = 1;
	skyboxRootsig.mParameters[4].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;





	skyboxRootsig.initialize(render.mDevice);


	skyBoxshader.shaders[VS].load("Shaders/SkyBoxDepthHDR.hlsl", "VSMain", VS);
	skyBoxshader.shaders[PS].load("Shaders/SkyBoxDepthHDR.hlsl", "PSMain", PS);

	skyBoxPipeline.createGraphicsPipeline(render.mDevice, skyboxRootsig, skyBoxshader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState());




	//IrradianceMap.createCubeRenderTargets(render.mDevice, IrrWidth, IrrHeight, 1,1, CUBE_RENDERTAERGET_TYPE_RENDERTARGET, rtvheap, srvheap, D3D12_RESOURCE_FLAG_NONE, DXGI_FORMAT_R32G32B32A32_FLOAT);
	IrradianceMap.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, IrrWidth, IrrHeight, 1, 1, TEXTURE_SRV_TYPE_CUBE, TEXTURE_USAGE_SRV_RTV);
//	SpecularMap.createCubeRenderTargets(render.mDevice, SpecWidth, SpecHeight, 1, specularlod, CUBE_RENDERTAERGET_TYPE_RENDERTARGET, rtvheap, srvheap, D3D12_RESOURCE_FLAG_NONE, DXGI_FORMAT_R32G32B32A32_FLOAT);
	SpecularMap.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, SpecWidth, SpecHeight, 1, specularlod, TEXTURE_SRV_TYPE_CUBE, TEXTURE_USAGE_SRV_RTV, TEXTURE_ALL_MIPS_USE_ALL);
	RenderTargetFormat brdfintformat(DXGI_FORMAT_R32G32_FLOAT);
//	BRDFIntergrateMap.createRenderTargets(render.mDevice, BRDFIntWidth, BRDFIntWidth, brdfintformat, rtvheap, srvheap);
	BRDFIntergrateMap.CreateTexture(render, srvheap, brdfintformat.mRenderTargetFormat[0], BRDFIntWidth, BRDFIntWidth, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);


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
	Buddha.mPosition[0].setScale(1.5, 1.5, 1.5);
	Buddha.mPosition[0].setPosition(0, -5, 0);
	Buddha.mPosition[0].CacNewTransform();
	Buddha.mBufferData[0].mMatrices = Buddha.mPosition[0].getMatrices();
	Buddha.mBufferData[0].mMaterial.mAlbedo = glm::vec3(1.00, 0.71, 0.29);
	Buddha.mBufferData[0].mMaterial.mRoughness = 1.0;
	Buddha.mBufferData[0].mMaterial.mMetallic = 1.0;


	groundimport.ReadFile("Assets/cube.obj", aiProcessPreset_TargetRealtime_Fast);
	aiMesh* ground = groundimport.GetScene()->mMeshes[0];
	Cube.mVertexBufferData.createVertexBuffer(render.mDevice, ground->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	Cube.mNormalBuffer.createVertexBuffer(render.mDevice, ground->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	std::vector<unsigned int> groundindexdata;
	groundindexdata.resize(ground->mNumFaces * 3);
	for (int i = 0; i < ground->mNumFaces; i++)
	{
		groundindexdata[i * 3] = ground->mFaces[i].mIndices[0];
		groundindexdata[i * 3 + 1] = ground->mFaces[i].mIndices[1];
		groundindexdata[i * 3 + 2] = ground->mFaces[i].mIndices[2];
	}
	Cube.mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * ground->mNumFaces * 3);
	Cube.indexCount = ground->mNumFaces * 3;
	Cube.mNum = 1;
	Cube.mBufferData.resize(Cube.mNum);
	Cube.mPosition.resize(Cube.mNum);
	Cube.mStructeredBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Cube.mNum, STRUCTERED_BUFFER_TYPE_READ);
	//Cube.mPosition[0].setAngle(-90, 0, 0);
	Cube.mPosition[0].setScale(50, 1, 50);
	Cube.mPosition[0].setPosition(0, -6, 0);
	Cube.mPosition[0].CacNewTransform();
	Cube.mBufferData[0].mMatrices = Cube.mPosition[0].getMatrices();
	Cube.mBufferData[0].mMaterial.mAlbedo = glm::vec3(0.4, 0.4, 0.4);
	Cube.mBufferData[0].mMaterial.mRoughness = 0.1;
	Cube.mBufferData[0].mMaterial.mMetallic = 0.0;





	// load sphere data

	import.ReadFile("Assets/bunny.obj", aiProcessPreset_TargetRealtime_Fast);
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

	// initialize Bunnys position data
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
		Bunnys.mPosition[i].setPosition(radious*cos(i*radian), heightgap*height-1.5, radious*sin(i*radian));
		Bunnys.mPosition[i].setScale(1.5, 1.5, 1.5);
		//Bunnys.mPosition[i].setAngle(-90, 0, 0);
		Bunnys.mPosition[i].CacNewTransform();
		Bunnys.mBufferData[i].mMatrices = Bunnys.mPosition[i].getMatrices();
		Bunnys.mBufferData[i].mMaterial.mAlbedo = glm::vec3(0.1, 0.01, 0.61);
		Bunnys.mBufferData[i].mMaterial.mRoughness = (rowpos)*rough;
		Bunnys.mBufferData[i].mMaterial.mMetallic = height*metalic;

	}




	lightDataBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(PointLightData), lightcount, STRUCTERED_BUFFER_TYPE_READ);

	lightlist.resize(lightcount);


	PointLight light;
	for (int i = 0; i < lightcount; ++i)
	{
		light.setRadius(distributionradius(generator));
		light.setColor(distributionlightcolor(generator), distributionlightcolor(generator), distributionlightcolor(generator));


		//	light.setColor(0, 0.0, 200.0);
		light.setPosition(distributionXZ(generator), 3, distributionXZ(generator));
		lightlist[i] = *light.getLightData();
	}



	float* skyboxdata[6];

	//skyboxdata[0] = stbi_loadf("Assets/Textures/milk0003.hdr", &width, &height, &bpp, 4);
	//skyboxdata[1] = stbi_loadf("Assets/Textures/milk0001.hdr", &width, &height, &bpp, 4);
	//skyboxdata[2] = stbi_loadf("Assets/Textures/milk0005.hdr", &width, &height, &bpp, 4);
	//skyboxdata[3] = stbi_loadf("Assets/Textures/milk0004.hdr", &width, &height, &bpp, 4);
	//skyboxdata[4] = stbi_loadf("Assets/Textures/milk0006.hdr", &width, &height, &bpp, 4);
	//skyboxdata[5] = stbi_loadf("Assets/Textures/milk0002.hdr", &width, &height, &bpp, 4);

	Image img[6];
	img[0].load("Assets/Textures/GravelPlaza0003.hdr", 5);
	img[1].load("Assets/Textures/GravelPlaza0001.hdr", 5);
	img[2].load("Assets/Textures/GravelPlaza0005.hdr", 5);
	img[3].load("Assets/Textures/GravelPlaza0004.hdr", 5);
	img[4].load("Assets/Textures/GravelPlaza0006.hdr", 5);
	img[5].load("Assets/Textures/GravelPlaza0002.hdr", 5);





	////	unsigned char* rgb = stbi_load("Assets/Textures/front.jpg", &width, &height, &bpp, 4);

	skyboxdata[0] = (float *)img[0].mData;
	skyboxdata[1] = (float *)img[1].mData;
	skyboxdata[2] = (float *)img[2].mData;
	skyboxdata[3] = (float *)img[3].mData;
	skyboxdata[4] = (float *)img[4].mData;
	skyboxdata[5] = (float *)img[5].mData;

	//img[0].load("Assets/Textures/right.jpg", 10);
	//img[1].load("Assets/Textures/left.jpg", 10);
	//img[2].load("Assets/Textures/top.jpg", 10);
	//img[3].load("Assets/Textures/bottom.jpg", 10);
	//img[4].load("Assets/Textures/back.jpg", 10);
	//img[5].load("Assets/Textures/front.jpg", 10);


	//	skyBox.CreateTexture(render.mDevice, DXGI_FORMAT_R8G8B8A8_UNORM, img[0].mWidth, img[0].mHeight, 6, true);
	//skyBox.CreateTexture(render.mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, img[0].mWidth, img[0].mHeight, 6, true, 5);

	skyBox.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, img[0].mWidth, img[0].mHeight, 1, 5, TEXTURE_SRV_TYPE_CUBE);

//	skyBox.addSahderResorceView(srvheap);




	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(Bunnys.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Bunnys.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Bunnys.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Cube.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Cube.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Cube.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Cube.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(lightDataBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(skyBox, D3D12_RESOURCE_STATE_COPY_DEST,true);




	cmdlist.updateBufferData(Bunnys.mVertexBufferData, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Bunnys.mIndexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Bunnys.mNormalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Bunnys.mStructeredBuffer, Bunnys.mBufferData.data(), Bunnys.mNum * sizeof(InstancedInformation));

	cmdlist.updateBufferData(Buddha.mVertexBufferData, buddha->mVertices, buddha->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Buddha.mIndexBuffer, buddhaindexdata.data(), buddha->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Buddha.mNormalBuffer, buddha->mNormals, buddha->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Buddha.mStructeredBuffer, Buddha.mBufferData.data(), Buddha.mNum * sizeof(InstancedInformation));

	cmdlist.updateBufferData(Cube.mVertexBufferData, ground->mVertices, ground->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Cube.mIndexBuffer, groundindexdata.data(), ground->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Cube.mNormalBuffer, ground->mNormals, ground->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Cube.mStructeredBuffer, Cube.mBufferData.data(), Cube.mNum * sizeof(InstancedInformation));

	cmdlist.updateBufferData(lightDataBuffer, lightlist.data(), lightlist.size() * sizeof(PointLightData));
	cmdlist.updateTextureCubeData(skyBox, (void const **)skyboxdata);

	cmdlist.resourceTransition(Bunnys.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Bunnys.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Bunnys.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceTransition(Buddha.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Buddha.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Buddha.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Buddha.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceTransition(Cube.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Cube.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Cube.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Cube.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdlist.resourceTransition(lightDataBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceTransition(skyBox, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);



	cmdlist.resourceTransition(GBuffer[0], D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceTransition(GBuffer[2], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,true);


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
void prepareMap()
{
	ShaderSet irrshaders;
	irrshaders.shaders[VS].load("Shaders/IrradianceCubeMap.hlsl", "VSMain", VS);
	irrshaders.shaders[PS].load("Shaders/IrradianceCubeMap.hlsl", "PSMain", PS);
	irrshaders.shaders[GS].load("Shaders/IrradianceCubeMap.hlsl", "GSMain", GS);

	ShaderSet specshaders;
	specshaders.shaders[VS].load("Shaders/SpecularCubeMap.hlsl", "VSMain", VS);
	specshaders.shaders[PS].load("Shaders/SpecularCubeMap.hlsl", "PSMain", PS);
	specshaders.shaders[GS].load("Shaders/SpecularCubeMap.hlsl", "GSMain", GS);

	ShaderSet brdfintshaders;
	brdfintshaders.shaders[VS].load("Shaders/IntergrateBRDF.hlsl", "VSMain", VS);
	brdfintshaders.shaders[PS].load("Shaders/IntergrateBRDF.hlsl", "PSMain", PS);


	struct Consts
	{
		unsigned int face;
		float roughness;
	} consts;

	


	RootSignature mapGenRoot;

	mapGenRoot.mParameters.resize(4);
	mapGenRoot.mParameters[0].mType = PARAMETERTYPE_CBV;
	mapGenRoot.mParameters[0].mResCounts = 1;
	mapGenRoot.mParameters[0].mBindSlot = 0;
	mapGenRoot.mParameters[0].mResource = &cameraBuffer;
	mapGenRoot.mParameters[1].mType = PARAMETERTYPE_SRV;
	mapGenRoot.mParameters[1].mResCounts = 1;
	mapGenRoot.mParameters[1].mBindSlot = 0;
	mapGenRoot.mParameters[1].mResource = &skyBox;
	mapGenRoot.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	mapGenRoot.mParameters[2].mType = PARAMETERTYPE_SAMPLER;
	mapGenRoot.mParameters[2].mResCounts = 1;
	mapGenRoot.mParameters[2].mBindSlot = 0;
	mapGenRoot.mParameters[2].mSampler = &sampler;
	mapGenRoot.mParameters[3].mType = PARAMETERTYPE_ROOTCONSTANT;
	mapGenRoot.mParameters[3].mResCounts = 2;
	mapGenRoot.mParameters[3].mBindSlot = 1;
	mapGenRoot.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_ALL;



	mapGenRoot.initialize(render.mDevice);



	RootSignature emptyroot;
	emptyroot.initialize(render.mDevice);


	Pipeline IrraidancePipeline,SpecularPipeline,BRDFIntPipeline;
	RenderTargetFormat retformat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	RenderTargetFormat brdfintformat(DXGI_FORMAT_R32G32_FLOAT);
	IrraidancePipeline.createGraphicsPipeline(render.mDevice, mapGenRoot, irrshaders, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);
	SpecularPipeline.createGraphicsPipeline(render.mDevice, mapGenRoot, specshaders, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);
	BRDFIntPipeline.createGraphicsPipeline(render.mDevice, emptyroot, brdfintshaders, brdfintformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);
	ViewPort tempviewport;
	Scissor tempscissor;
	tempviewport.setup(0.0f, 0.0f, (float)IrrWidth, (float)IrrHeight);
	tempscissor.setup(0, IrrWidth, 0, IrrHeight);



	Camera views[6];
	views[0].setTarget(-1.0, 0.0, 0.0);
	views[0].setUp(0.0, -1.0, 0.0);

	views[1].setTarget(1.0, 0.0, 0.0);
	views[1].setUp(0.0, -1.0, 0.0);

	views[2].setTarget(0.0, -1.0, 0.0);
	views[2].setUp(0.0, 0.0, 1.0);

	views[3].setTarget(0.0, 1.0, 0.0);
	views[3].setUp(0.0, 0.0, -1.0);

	views[4].setTarget(0.0, 0.0, -1.0);
	views[4].setUp(0.0, -1.0, 0.0);
	views[5].setTarget(0.0, 0.0, 1.0);
	views[5].setUp(0.0, -1.0, 0.0);


	for (int i = 0; i < 6; ++i)
	{
		views[i].setFOV(90);
		views[i].setRatio(1.0);
		views[i].updateViewProj();

	}






		for (consts.face = 0; consts.face < 6; ++consts.face)
		{


			cmdalloc.reset();
			cmdlist.reset(IrraidancePipeline);

			//		cmdlist.cubeRenderTargetBarrier(IrradianceMap, D3D12_RESOURCE_STATE_GENERIC_READ
			//		, D3D12_RESOURCE_STATE_RENDER_TARGET);
			cmdlist.resourceTransition(IrradianceMap, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			cmdlist.setViewPort(tempviewport);
			cmdlist.setScissor(tempscissor);
			cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);
			cmdlist.bindGraphicsRootSigature(mapGenRoot);
			cameraBuffer.updateBufferfromCpu(views[consts.face].getMatrix(), sizeof(ViewProjection)); // bad use, this is reason why need to wait 6 times for execute gpu command
			cmdlist.bindRenderTargetsOnly(IrradianceMap);
			cmdlist.bindGraphicsConstant(3, &consts);
			cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdlist.bindIndexBuffer(Cube.mIndexBuffer);
			cmdlist.bindVertexBuffers(Cube.mVertexBufferData, Cube.mNormalBuffer);
			cmdlist.drawIndexedInstanced(Cube.indexCount, 1, 0, 0);

			cmdlist.resourceTransition(IrradianceMap, D3D12_RESOURCE_STATE_GENERIC_READ,true);

			/*if (consts.face != 5)
				cmdlist.cubeRenderTargetBarrier(IrradianceMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
*/
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


		}


	tempviewport.setup(0.0f, 0.0f, (float)SpecWidth, (float)SpecHeight);
	tempscissor.setup(0, SpecWidth, 0, SpecHeight);

	consts.roughness = 0.2;

	for (UINT  lod = 0; lod < specularlod; lod++)
	{

		tempviewport.setup(0.0f, 0.0f, (float)SpecWidth/pow(2,lod), (float)SpecHeight / pow(2, lod));
		tempscissor.setup(0, SpecWidth / pow(2, lod), 0, SpecHeight / pow(2, lod));


		consts.roughness = float(lod) / float(specularlod);
		for (consts.face = 0; consts.face < 6; ++consts.face)
		{

			cmdalloc.reset();
			cmdlist.reset(SpecularPipeline);

			//		cmdlist.cubeRenderTargetBarrier(IrradianceMap, D3D12_RESOURCE_STATE_GENERIC_READ
			//		, D3D12_RESOURCE_STATE_RENDER_TARGET);
			cmdlist.resourceTransition(SpecularMap, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			cmdlist.setViewPort(tempviewport);
			cmdlist.setScissor(tempscissor);
			cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);
			cmdlist.bindGraphicsRootSigature(mapGenRoot);
			cameraBuffer.updateBufferfromCpu(views[consts.face].getMatrix(), sizeof(ViewProjection));
			cmdlist.bindRenderTargetsOnly(SpecularMap,lod);
			cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdlist.bindGraphicsConstant(3, &consts);
			cmdlist.bindIndexBuffer(Cube.mIndexBuffer);
			cmdlist.bindVertexBuffers(Cube.mVertexBufferData, Cube.mNormalBuffer);
			cmdlist.drawIndexedInstanced(Cube.indexCount, 1, 0, 0);

			
			cmdlist.resourceTransition(SpecularMap, D3D12_RESOURCE_STATE_GENERIC_READ, true);

			/*if (i != 5 && lod!=specularlod-1)
				cmdlist.cubeRenderTargetBarrier(SpecularMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
*/
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


		}
	}



	tempviewport.setup(0.0f, 0.0f, (float)BRDFIntWidth, (float)BRDFIntHeight);
	tempscissor.setup(0, BRDFIntWidth, 0, BRDFIntHeight);


	cmdalloc.reset();
	cmdlist.reset(BRDFIntPipeline);

			cmdlist.resourceTransition(BRDFIntergrateMap
			, D3D12_RESOURCE_STATE_RENDER_TARGET,true);

	cmdlist.setViewPort(tempviewport);
	cmdlist.setScissor(tempscissor);
	cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);
	cmdlist.bindGraphicsRootSigature(emptyroot);
	cmdlist.bindRenderTargetsOnly(BRDFIntergrateMap);
	//cameraBuffer.updateBufferfromCpu(views[i].getMatrix(), sizeof(ViewProjection));
//	cmdlist.bindCubeRenderTarget(IrradianceMap, i);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist.drawInstance(3, 1, 0, 0);
	//cmdlist.bindIndexBuffer(Cube.mIndexBuffer);
	//cmdlist.bindVertexBuffers(Cube.mVertexBufferData, Cube.mNormalBuffer);
	//cmdlist.drawIndexedInstanced(Cube.indexCount, 1, 0, 0);

	cmdlist.resourceTransition(BRDFIntergrateMap, D3D12_RESOURCE_STATE_GENERIC_READ,true);

	

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

	mapGenRoot.realease();
	IrraidancePipeline.release();
	SpecularPipeline.release();
	BRDFIntPipeline.release();
	emptyroot.realease();
}



void releaseRender()
{

	BRDFIntergrateMap.release();
	SpecularMap.release();
	IrradianceMap.release();

	skyBoxPipeline.release();
	skyboxRootsig.realease();
	skyBox.release();
	
	localLightpPipeline.release();
	lightDataBuffer.release();

	defferedrootsig.realease();
	defferedpipeline.release();
	quadpipeline.release();
	quadrootsig.realease();
	samplerheap.release();
	
	GBuffer[2].release();
	GBuffer[1].release();
	GBuffer[0].release();
	rtvheap.release();
	dsvheap.release();

	Cube.mNormalBuffer.release();
	Cube.mVertexBufferData.release();
	Cube.mStructeredBuffer.release();
	Cube.mIndexBuffer.release();
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



	//radian = 2 * 3.14159f / Bunnys.mNum;
	rotationoffset += rotationspeed;
	//for (int i = 0; i < Bunnys.mNum; ++i)
	//{
	//	int height = i / rowcount;
	//	int rowpos = i % rowcount;
	//	Bunnys.mPosition[i].setPosition(radious*cos(rowpos*radian + rotationoffset), heightgap*height - (collomcount*heightgap) / 2, radious*sin(rowpos*radian + rotationoffset));

	//	//	Bunnys.mPosition[i].setPosition(rowpos*1.5, height, 5);
	//	//Bunnys.mPosition[i].setAngle(0, (rowpos*radian+ rotationoffset)/3.14159*180, 0);
	//	Bunnys.mPosition[i].CacNewTransform();
	//	Bunnys.mBufferData[i].mMatrices = Bunnys.mPosition[i].getMatrices();
	//}


	for (int i = 0; i < lightcount; ++i)
	{

		lightlist[i].mPosition.x += distributionmove(generator);


		if (lightlist[i].mPosition.x >= 60)
		{
			lightlist[i].mPosition.x = -60;
			//	lightlist[i].mColor.r = distributionlightcolor(generator);
			//	lightlist[i].mColor.g = distributionlightcolor(generator);
			//	lightlist[i].mColor.b = distributionlightcolor(generator);
		}
	}

}
void onrender()
{

	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(pipeline);

	cmdlist.resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_COPY_DEST);


	cmdlist.resourceTransition(lightDataBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.updateBufferData(Bunnys.mStructeredBuffer, Bunnys.mBufferData.data(), Bunnys.mNum * sizeof(InstancedInformation));
	cmdlist.updateBufferData(lightDataBuffer, lightlist.data(), lightlist.size() * sizeof(PointLightData));

	cmdlist.resourceTransition(lightDataBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceTransition(Bunnys.mStructeredBuffer, D3D12_RESOURCE_STATE_GENERIC_READ,true);

	cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	{ // GBuffer - Pass
		cmdlist.setViewPort(viewport);
		cmdlist.setScissor(scissor);
		cmdlist.bindGraphicsRootSigature(rootsig);


		//cmdlist.renderTargetBarrier(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		cmdlist.resourceTransition(GBuffer[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdlist.resourceTransition(GBuffer[2] , D3D12_RESOURCE_STATE_DEPTH_WRITE,true);

		cmdlist.bindRenderTargetsDepthStencil(GBuffer[0], GBuffer[1], GBuffer[2]);
		cmdlist.clearRenderTarget(GBuffer[0]);
		cmdlist.clearRenderTarget(GBuffer[1]);
		cmdlist.clearDepthStencil(GBuffer[2]);


		cmdlist.bindIndexBuffer(Bunnys.mIndexBuffer);
		cmdlist.bindVertexBuffers(Bunnys.mVertexBufferData, Bunnys.mNormalBuffer);
		cmdlist.drawIndexedInstanced(Bunnys.indexCount, Bunnys.mNum, 0, 0);

		cmdlist.bindGraphicsResource(1, Buddha.mStructeredBuffer);
		cmdlist.bindIndexBuffer(Buddha.mIndexBuffer);
		cmdlist.bindVertexBuffers(Buddha.mVertexBufferData, Buddha.mNormalBuffer);
		cmdlist.drawIndexedInstanced(Buddha.indexCount, Buddha.mNum, 0, 0);


		cmdlist.bindGraphicsResource(1, Cube.mStructeredBuffer);
		cmdlist.bindIndexBuffer(Cube.mIndexBuffer);
		cmdlist.bindVertexBuffers(Cube.mVertexBufferData, Cube.mNormalBuffer);
		cmdlist.drawIndexedInstanced(Cube.indexCount, Cube.mNum, 0, 0);


		cmdlist.resourceTransition(GBuffer[0], D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdlist.resourceTransition(GBuffer[2], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	}
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex]);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	{ //deffered pass
		cmdlist.bindGraphicsRootSigature(defferedrootsig);
		cmdlist.bindPipeline(defferedpipeline);
		cmdlist.bindGraphicsResource(6, IrradianceMap);
		cmdlist.bindGraphicsResource(7, SpecularMap);
		cmdlist.bindGraphicsResource(8, BRDFIntergrateMap);


		cmdlist.drawInstance(3, 1, 0, 0);

	/*	cmdlist.bindPipeline(localLightpPipeline);
		cmdlist.bindIndexBuffer(Bunnys.mIndexBuffer);
		cmdlist.bindVertexBuffers(Bunnys.mVertexBufferData, Bunnys.mNormalBuffer);
		cmdlist.drawIndexedInstanced(Bunnys.indexCount, lightcount, 0, 0);
*/
		// skybox

		cmdlist.bindGraphicsRootSigature(skyboxRootsig);
		cmdlist.bindPipeline(skyBoxPipeline);
		cmdlist.bindGraphicsConstant(4, windowSize);
		cmdlist.bindGraphicsResource(1, SpecularMap);
		cmdlist.bindIndexBuffer(Cube.mIndexBuffer);
		cmdlist.bindVertexBuffers(Cube.mVertexBufferData);
		cmdlist.drawIndexedInstanced(Cube.indexCount, Cube.mNum, 0, 0);

	}
	{ // debug pass
	/*	cmdlist.bindGraphicsRootSigature(quadrootsig);
		cmdlist.bindPipeline(quadpipeline);

		debugviewport.setup(0.0f, 0.0f, (float)400, (float)400);
		cmdlist.setViewPort(debugviewport);
		cmdlist.bindGraphicsResource(0, BRDFIntergrateMap.mRenderBuffers[0]);
		cmdlist.drawInstance(3, 1, 0, 0);*/
		//debugviewport.setup(0.0f, (float)windows.mHeight / 4, (float)windows.mWidth / 4, (float)windows.mHeight / 4);
		//cmdlist.setViewPort(debugviewport);
		//cmdlist.bindGraphicsResource(0, GBuffer.mRenderBuffers[1]);
		//cmdlist.drawInstance(3, 1, 0, 0);
		//debugviewport.setup(0.0f, 2 * (float)windows.mHeight / 4, (float)windows.mWidth / 4, (float)windows.mHeight / 4);
		//cmdlist.setViewPort(debugviewport);
		//cmdlist.bindGraphicsResource(0, GBuffer.mDepthBuffer[0]);
		//cmdlist.drawInstance(3, 1, 0, 0);



	}
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
	windows.initialize(1600, 900, "Imaged Based Lighting");
	windows.openWindow();
	glfwSetCursorPosCallback(windows.mWindow, cursor_pos_callback);
	glfwSetMouseButtonCallback(windows.mWindow, mouse_button_callback);
	glfwSetScrollCallback(windows.mWindow, scroll_callback);
	glfwSetKeyCallback(windows.mWindow, key_callback);

	int limit = 10000;
	int count = 0;
	initializeRender();
	loadAsset();
	prepareMap();
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