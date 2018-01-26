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
#include "GameCamera.h"
#include "Object.h"
#include "DirectionLight.h"
#include "GaussionCurve.h"
#include <chrono>
#include <random>
#include <algorithm>
#include <array>
using namespace std;
Render render;
CommandAllocator cmdalloc;
CommandList cmdlist;
Window windows;
UINT frameIndex;
Buffer vertexBuffer;
Buffer normalBuffer;
Buffer indexBuffer;
Buffer cameraBuffer;
RootSignature rootsig;
ViewPort viewport;
Scissor scissor;
DescriptorHeap srvheap;
DescriptorHeap rtvheap;
DescriptorHeap dsvheap;
static uint32_t swapChainCount = 3;
Fence fence;
ShaderSet shaderset;
Assimp::Importer import;
SpecCamera camera;
GameCamera gamecamera;
aiScene const * scene = nullptr;
aiMesh* mesh = nullptr;
float curxpos;
float curypos;
float curxoffet;
float curyoffet;
bool press = false;
bool shiftpress = false;
struct IndirectDrawCmd
{
	VertexBufferView position;
	VertexBufferView uv;
	VertexBufferView normal;
	VertexBufferView tangent;
	VertexBufferView bitangent;
	IndexBufferView index;
	unsigned int objectindex;
	DrawIndexedArgument argument;
};
struct IndirectPointLightDrawCmd
{
	unsigned int lightindex;
	DrawArgument argument;
};
struct IndirecShadowDrawCmd  // only need position
{
	VertexBufferView position;
	IndexBufferView index;
	unsigned int objectindex;
	DrawIndexedArgument argument;
};

CommandSignature geomCmdSig;
RootSignature geoCmdGenRootSig;
//Prototype of dynmaic index with exectue indirect pipeline 
std::vector<Mesh> meshList;
std::vector<IndirectMeshData> indirectMeshList;
std::vector<Material> materialList;
std::vector<Matrices> matricesList;
std::vector<Object> objectList;


Buffer indirecrtMeshBuffer;
Buffer materialBuffer;
Buffer matricesBuffer;
Buffer objectBuffer;
Buffer indirectGeoCmdBuffer;
int maxmeshcount = 1000;
int maxobjectcount = 1000;
Pipeline GeoCmdGenPipeline;
RootSignature geoDrawSig;
Pipeline GeoDrawPipeline;
Texture DefualtTexture;
vector<Texture> textureList;
int maxtexturecount = 1000;
vector<Image> imageList;
int texturecount = 0;

DescriptorHeap samplerheap;
Sampler matsampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
Sampler gbuffersampler;
//RenderTarget GBuffer;
vector<Texture> GBuffer;


DirectionLight sunlight;
float spint = 121;
float tilt = -80.0f;
float sunintensity = 5.0f;
Buffer lightBuffer;
Buffer shadowIndirectCmdBuffer;
CommandSignature shadowCmdSig;
bool showcascaded = false;
bool usesmsr = true;
unsigned int shadowcount = 4;
unsigned int shadowwidth = 1024;
unsigned int shadowheight = 1024;
Texture cascadedShadowMap;
Texture disconMap;
RootSignature shadowRootsig;
Pipeline shadowPipeline;
ViewPort shadowviewport;
Scissor shadowscissor;




RootSignature lightRootsig;
Pipeline lightPipeline;

RootSignature depthDiscRootsig;
Pipeline depthDiscPipeline;




Texture HDRBuffer;  // another positon why seperate textue and rendertarget is wrong, should use usage approach while creating resource
Texture BloomBuffer;
Texture TempBuffer;

GaussionWeight gaussdata;

GaussionCurve curve(10, 3);
float bloomthreash = 1.0f;
RootSignature brightextsig;
RootSignature gaussiansig;
Buffer gaussianconst;
Sampler pointsam(D3D12_FILTER_MIN_MAG_MIP_POINT);
Pipeline brightExtractPipeline;
Pipeline gaussianPipeline;
Pipeline combinePipe;
RootSignature combinesig;

std::chrono::high_resolution_clock::time_point pre;
void initializeRender()
{


	//Camera cam;

//	cout << sizeof(IndirecShadowDrawCmd) << endl;
	//	system("pause");

	render.initialize();
	RenderTargetFormat retformat;
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
	fence.initialize(render);
	cmdalloc.initialize(render.mDevice);
	cmdlist.initial(render.mDevice, cmdalloc);



	srvheap.ininitialize(render.mDevice, 1);
	rtvheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	DXGI_FORMAT formats[3];
	formats[0] = DXGI_FORMAT_R16G16_FLOAT; // normal
	formats[1] = DXGI_FORMAT_R8G8B8A8_UNORM; // color + roughness
	formats[2] = DXGI_FORMAT_R8G8B8A8_UNORM; // emmersive + metalic
	RenderTargetFormat gBufferFormat(3, formats, true);
//	GBuffer.createRenderTargets(render.mDevice, windows.mWidth, windows.mHeight, gBufferFormat, rtvheap, dsvheap, srvheap);
	GBuffer.resize(4);
	GBuffer[0].CreateTexture(render, srvheap, gBufferFormat.mRenderTargetFormat[0], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);
	GBuffer[1].CreateTexture(render, srvheap, gBufferFormat.mRenderTargetFormat[1], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);
	GBuffer[2].CreateTexture(render, srvheap, gBufferFormat.mRenderTargetFormat[2], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);
	GBuffer[3].CreateTexture(render, srvheap, gBufferFormat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_DSV);

	//DXGI_FORMAT hdrformat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//RenderTargetFormat hdrFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
//	HDRBuffer.createRenderTargets(render.mDevice, windows.mWidth, windows.mHeight, hdrFormat, rtvheap, srvheap);

	HDRBuffer.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);

	BloomBuffer.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, windows.mWidth / 2, windows.mHeight / 2, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);
	//BloomBuffer.addUnorderedAccessView(srvheap);
	//BloomBuffer.addSahderResorceView(srvheap);


	TempBuffer.CreateTexture(render,srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, windows.mWidth / 2, windows.mHeight / 2, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);
//	TempBuffer.addUnorderedAccessView(srvheap);
//	TempBuffer.addSahderResorceView(srvheap);


	//FinalBuffer.createRenderTargets(render.mDevice, windows.mWidth, windows.mHeight, retformat, rtvheap, srvheap);
	//FinalBuffer.CreateTexture(render.mDevice, DXGI_FORMAT_R8G8B8A8_UNORM, windows.mWidth, windows.mHeight, 1, false, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);


	samplerheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	matsampler.createSampler(samplerheap);
	gbuffersampler.createSampler(samplerheap);
	pointsam.createSampler(samplerheap);

	rootsig.mParameters.resize(1);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;



	rootsig.initialize(render.mDevice);

	



	geoCmdGenRootSig.mParameters.resize(4);
	geoCmdGenRootSig.mParameters[0].mType = PARAMETERTYPE_UAV; // indirect command + shadow command
	geoCmdGenRootSig.mParameters[0].mResCounts = 2;
	geoCmdGenRootSig.mParameters[0].mBindSlot = 0;
	geoCmdGenRootSig.mParameters[0].mResource = &indirectGeoCmdBuffer;
	geoCmdGenRootSig.mParameters[1].mType = PARAMETERTYPE_SRV; //objct + mac + mesh
	geoCmdGenRootSig.mParameters[1].mResCounts = 3;
	geoCmdGenRootSig.mParameters[1].mBindSlot = 0;
	geoCmdGenRootSig.mParameters[1].mResource = &objectBuffer;
	geoCmdGenRootSig.mParameters[2].mType = PARAMETERTYPE_CBV;
	geoCmdGenRootSig.mParameters[2].mResCounts = 1;
	geoCmdGenRootSig.mParameters[2].mBindSlot = 0;
	geoCmdGenRootSig.mParameters[2].mResource = &cameraBuffer;
	geoCmdGenRootSig.mParameters[3].mType = PARAMETERTYPE_CBV;
	geoCmdGenRootSig.mParameters[3].mResCounts = 1;
	geoCmdGenRootSig.mParameters[3].mBindSlot = 1;
	geoCmdGenRootSig.mParameters[3].mResource = &lightBuffer;



	geoCmdGenRootSig.initialize(render.mDevice);

	ShaderSet geocmdgencs;
	geocmdgencs.shaders[CS].load("Shaders/CascadedShadowMaps/ObjectFrsutumCulling.hlsl", "CSMain", CS);
	GeoCmdGenPipeline.createComputePipeline(render.mDevice, geoCmdGenRootSig, geocmdgencs);


	geoDrawSig.mParameters.resize(7);
	geoDrawSig.mParameters[0].mType = PARAMETERTYPE_ROOTCONSTANT; // object index
	geoDrawSig.mParameters[0].mResCounts = 1;
	geoDrawSig.mParameters[0].mBindSlot = 1;
	geoDrawSig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	geoDrawSig.mParameters[1].mType = PARAMETERTYPE_SRV; // objct 
	geoDrawSig.mParameters[1].mResCounts = 1;
	geoDrawSig.mParameters[1].mBindSlot = 0;
	geoDrawSig.mParameters[1].mResource = &objectBuffer;
	geoDrawSig.mParameters[2].mType = PARAMETERTYPE_SRV; // matrices
	geoDrawSig.mParameters[2].mResCounts = 1;
	geoDrawSig.mParameters[2].mBindSlot = 1;
	geoDrawSig.mParameters[2].mResource = &matricesBuffer;
	geoDrawSig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	geoDrawSig.mParameters[3].mType = PARAMETERTYPE_SRV; // material
	geoDrawSig.mParameters[3].mResCounts = 1;
	geoDrawSig.mParameters[3].mBindSlot = 2;
	geoDrawSig.mParameters[3].mResource = &materialBuffer;
	geoDrawSig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	geoDrawSig.mParameters[4].mType = PARAMETERTYPE_CBV;
	geoDrawSig.mParameters[4].mResCounts = 1;
	geoDrawSig.mParameters[4].mBindSlot = 0;
	geoDrawSig.mParameters[4].mResource = &cameraBuffer;
	geoDrawSig.mParameters[4].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	geoDrawSig.mParameters[5].mType = PARAMETERTYPE_SAMPLER;
	geoDrawSig.mParameters[5].mResCounts = 1;
	geoDrawSig.mParameters[5].mBindSlot = 0;
	geoDrawSig.mParameters[5].mSampler = &matsampler;
	geoDrawSig.mParameters[5].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	geoDrawSig.mParameters[6].mType = PARAMETERTYPE_SRV;
	geoDrawSig.mParameters[6].mResCounts = -1;
	geoDrawSig.mParameters[6].mBindSlot = 0;
	geoDrawSig.mParameters[6].mShaderSpace = 1;
	geoDrawSig.mParameters[6].rangeflag = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
	geoDrawSig.mParameters[6].mResource = &DefualtTexture;
	geoDrawSig.mParameters[6].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	geoDrawSig.initialize(render.mDevice);

	ShaderSet geoDrawShader;
	geoDrawShader.shaders[VS].load("Shaders/CascadedShadowMaps/CascadedGeom.hlsl", "VSMain", VS);
	geoDrawShader.shaders[PS].load("Shaders/CascadedShadowMaps/CascadedGeom.hlsl", "PSMain", PS);

	GeoDrawPipeline.createGraphicsPipeline(render.mDevice, geoDrawSig, geoDrawShader, gBufferFormat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT), VERTEX_LAYOUT_TYPE_SPLIT_ALL);


	geomCmdSig.mParameters.resize(8);
	geomCmdSig.mParameters[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	geomCmdSig.mParameters[0].VertexBuffer.Slot = 0;
	geomCmdSig.mParameters[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	geomCmdSig.mParameters[1].VertexBuffer.Slot = 1;
	geomCmdSig.mParameters[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	geomCmdSig.mParameters[2].VertexBuffer.Slot = 2;
	geomCmdSig.mParameters[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	geomCmdSig.mParameters[3].VertexBuffer.Slot = 3;
	geomCmdSig.mParameters[4].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	geomCmdSig.mParameters[4].VertexBuffer.Slot = 4;
	geomCmdSig.mParameters[5].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
	geomCmdSig.mParameters[6].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	geomCmdSig.mParameters[6].Constant.Num32BitValuesToSet = 1; // first paramenter in root sig
	geomCmdSig.mParameters[6].Constant.RootParameterIndex = 0;
	geomCmdSig.mParameters[6].Constant.DestOffsetIn32BitValues = 0;
	geomCmdSig.mParameters[7].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	geomCmdSig.initialize(render.mDevice, geoDrawSig);


	// shadow pass

	cascadedShadowMap.CreateTexture(render, srvheap, DXGI_FORMAT_R32_TYPELESS, shadowwidth, shadowheight, 8, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_DSV);
	disconMap.CreateTexture(render, srvheap, DXGI_FORMAT_R16G16B16A16_FLOAT, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);

	//RenderTargetFormat shadowformat(0, nullptr, true,false, DXGI_FORMAT_R16_TYPELESS);
	//cascadedShadowMap.createRenderTargets(render.mDevice, shadowwidth, shadowheight, shadowformat, dsvheap, srvheap);
	shadowviewport.setup(0.0f, 0.0f, (float)shadowwidth, (float)shadowheight);
	shadowscissor.setup(0, shadowwidth, 0, shadowheight);

	shadowRootsig.mParameters.resize(4);
	shadowRootsig.mParameters[0].mType = PARAMETERTYPE_ROOTCONSTANT; // object index
	shadowRootsig.mParameters[0].mResCounts = 1;
	shadowRootsig.mParameters[0].mBindSlot = 1;
	shadowRootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	shadowRootsig.mParameters[1].mType = PARAMETERTYPE_SRV; // objct 
	shadowRootsig.mParameters[1].mResCounts = 1;
	shadowRootsig.mParameters[1].mBindSlot = 0;
	shadowRootsig.mParameters[1].mResource = &objectBuffer;
	shadowRootsig.mParameters[2].mType = PARAMETERTYPE_SRV; // matrices
	shadowRootsig.mParameters[2].mResCounts = 1;
	shadowRootsig.mParameters[2].mBindSlot = 1;
	shadowRootsig.mParameters[2].mResource = &matricesBuffer;
	shadowRootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	shadowRootsig.mParameters[3].mType = PARAMETERTYPE_CBV;
	shadowRootsig.mParameters[3].mResCounts = 1;
	shadowRootsig.mParameters[3].mBindSlot = 0;
	shadowRootsig.mParameters[3].mResource = &lightBuffer;
	shadowRootsig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	shadowRootsig.initialize(render.mDevice);

	ShaderSet shadowshaders;
	shadowshaders.shaders[VS].load("Shaders/CascadedShadowMaps/CascadedShadowMap.hlsl", "VSMain", VS);
	shadowshaders.shaders[GS].load("Shaders/CascadedShadowMaps/CascadedShadowMap.hlsl", "GSMain", GS);

	RenderTargetFormat shadowformat(0, nullptr, true,false, DXGI_FORMAT_R32_TYPELESS);
	shadowPipeline.createGraphicsPipeline(render.mDevice, shadowRootsig, shadowshaders, shadowformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(0.0f,0.01f,0.1f, D3D12_CULL_MODE_FRONT), VERTEX_LAYOUT_TYPE_SPLIT_ALL);


	shadowCmdSig.mParameters.resize(4);
	shadowCmdSig.mParameters[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	shadowCmdSig.mParameters[0].VertexBuffer.Slot = 0;
	shadowCmdSig.mParameters[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
	shadowCmdSig.mParameters[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	shadowCmdSig.mParameters[2].Constant.Num32BitValuesToSet = 1; // first paramenter in root sig
	shadowCmdSig.mParameters[2].Constant.RootParameterIndex = 0;
	shadowCmdSig.mParameters[2].Constant.DestOffsetIn32BitValues = 0;
	shadowCmdSig.mParameters[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	shadowCmdSig.initialize(render.mDevice, shadowRootsig);


	// depth disc pass

	depthDiscRootsig.mParameters.resize(6);
	depthDiscRootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	depthDiscRootsig.mParameters[0].mResCounts = 1;
	depthDiscRootsig.mParameters[0].mBindSlot = 0;
	depthDiscRootsig.mParameters[0].mResource = &cameraBuffer;
	depthDiscRootsig.mParameters[1].mType = PARAMETERTYPE_CBV;
	depthDiscRootsig.mParameters[1].mResCounts = 1;
	depthDiscRootsig.mParameters[1].mBindSlot = 1;
	depthDiscRootsig.mParameters[1].mResource = &lightBuffer;
	depthDiscRootsig.mParameters[2].mType = PARAMETERTYPE_SRV;
	depthDiscRootsig.mParameters[2].mResCounts = 4;
	depthDiscRootsig.mParameters[2].mBindSlot = 1;
	depthDiscRootsig.mParameters[2].mResource = &GBuffer[0];
	depthDiscRootsig.mParameters[3].mType = PARAMETERTYPE_UAV;
	depthDiscRootsig.mParameters[3].mResCounts = 1;
	depthDiscRootsig.mParameters[3].mBindSlot = 0;
	depthDiscRootsig.mParameters[3].mResource = &disconMap;
	depthDiscRootsig.mParameters[4].mType = PARAMETERTYPE_SRV;
	depthDiscRootsig.mParameters[4].mResCounts = 1;
	depthDiscRootsig.mParameters[4].mBindSlot = 0;
	depthDiscRootsig.mParameters[4].mResource = &cascadedShadowMap;
	depthDiscRootsig.mParameters[5].mType = PARAMETERTYPE_SAMPLER;
	depthDiscRootsig.mParameters[5].mResCounts = 1;
	depthDiscRootsig.mParameters[5].mBindSlot = 0;
	depthDiscRootsig.mParameters[5].mSampler = &pointsam;
	depthDiscRootsig.initialize(render.mDevice);


	ShaderSet depthextdirshader;
	depthextdirshader.shaders[CS].load("Shaders/CascadedShadowMaps/SilhouetteEdgeExtract.hlsl", "CSMain", CS);
	depthDiscPipeline.createComputePipeline(render.mDevice, depthDiscRootsig, depthextdirshader);


	// light pass
	lightRootsig.mParameters.resize(7);
	lightRootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	lightRootsig.mParameters[0].mResCounts = 1;
	lightRootsig.mParameters[0].mBindSlot = 0;
	lightRootsig.mParameters[0].mResource = &cameraBuffer;
	lightRootsig.mParameters[1].mType = PARAMETERTYPE_CBV;
	lightRootsig.mParameters[1].mResCounts = 1;
	lightRootsig.mParameters[1].mBindSlot = 1;
	lightRootsig.mParameters[1].mResource = &lightBuffer;
	lightRootsig.mParameters[2].mType = PARAMETERTYPE_SRV;
	lightRootsig.mParameters[2].mResCounts = 4;
	lightRootsig.mParameters[2].mBindSlot = 1;
	lightRootsig.mParameters[2].mResource = &GBuffer[0];
	lightRootsig.mParameters[3].mType = PARAMETERTYPE_UAV;
	lightRootsig.mParameters[3].mResCounts = 1;
	lightRootsig.mParameters[3].mBindSlot = 0;
	lightRootsig.mParameters[3].mResource = &HDRBuffer;
	lightRootsig.mParameters[4].mType = PARAMETERTYPE_SRV;
	lightRootsig.mParameters[4].mResCounts = 1;
	lightRootsig.mParameters[4].mBindSlot = 0;
	lightRootsig.mParameters[4].mResource = &cascadedShadowMap;
	lightRootsig.mParameters[5].mType = PARAMETERTYPE_SAMPLER;
	lightRootsig.mParameters[5].mResCounts = 1;
	lightRootsig.mParameters[5].mBindSlot = 0;
	lightRootsig.mParameters[5].mSampler = &pointsam;
	lightRootsig.mParameters[6].mType = PARAMETERTYPE_SRV;
	lightRootsig.mParameters[6].mResCounts = 1;
	lightRootsig.mParameters[6].mBindSlot = 5;
	lightRootsig.mParameters[6].mResource = &disconMap;
	lightRootsig.initialize(render.mDevice);

	ShaderSet ambeintdirshader;
	ambeintdirshader.shaders[CS].load("Shaders/CascadedShadowMaps/AmbientandDirectionLight.hlsl", "CSMain", CS);
	lightPipeline.createComputePipeline(render.mDevice, lightRootsig, ambeintdirshader);


	

	//depthDiscPipeline


	brightextsig.mParameters.resize(4);
	brightextsig.mParameters[0].mType = PARAMETERTYPE_ROOTCONSTANT;
	brightextsig.mParameters[0].mResCounts = 1;
	brightextsig.mParameters[0].mBindSlot = 0;
	brightextsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	brightextsig.mParameters[1].mResCounts = 1;
	brightextsig.mParameters[1].mBindSlot = 0;
	brightextsig.mParameters[1].mResource = &HDRBuffer;
	brightextsig.mParameters[2].mType = PARAMETERTYPE_UAV;
	brightextsig.mParameters[2].mResCounts = 1;
	brightextsig.mParameters[2].mBindSlot = 0;
	brightextsig.mParameters[2].mResource = &BloomBuffer;
	brightextsig.mParameters[3].mType = PARAMETERTYPE_SAMPLER;
	brightextsig.mParameters[3].mResCounts = 1;
	brightextsig.mParameters[3].mBindSlot = 0;
	brightextsig.mParameters[3].mSampler = &pointsam;
	brightextsig.initialize(render.mDevice);


	// post process pass;
	ShaderSet brightshader;
	brightshader.shaders[CS].load("Shaders/CascadedShadowMaps/BrightExtract.hlsl", "CSMain", CS);
	brightExtractPipeline.createComputePipeline(render.mDevice, brightextsig, brightshader);



	gaussiansig.mParameters.resize(4);

	gaussiansig.mParameters[0].mType = PARAMETERTYPE_CBV;
	gaussiansig.mParameters[0].mResCounts = 1;
	gaussiansig.mParameters[0].mBindSlot = 0;
	gaussiansig.mParameters[0].mResource = &gaussianconst;
	gaussiansig.mParameters[1].mType = PARAMETERTYPE_ROOTCONSTANT;
	gaussiansig.mParameters[1].mResCounts = 1;
	gaussiansig.mParameters[1].mBindSlot = 1;
	gaussiansig.mParameters[2].mType = PARAMETERTYPE_SRV;
	gaussiansig.mParameters[2].mResCounts = 1;
	gaussiansig.mParameters[2].mBindSlot = 0;
	gaussiansig.mParameters[3].mType = PARAMETERTYPE_UAV;
	gaussiansig.mParameters[3].mResCounts = 1;
	gaussiansig.mParameters[3].mBindSlot = 0;

	gaussiansig.initialize(render.mDevice);


	ShaderSet gaussianshader;
	gaussianshader.shaders[CS].load("Shaders/CascadedShadowMaps/GaussianBlur.hlsl", "CSMain", CS);
	gaussianPipeline.createComputePipeline(render.mDevice, gaussiansig, gaussianshader);



	// final 

	combinesig.mParameters.resize(3);
	combinesig.mParameters[0].mType = PARAMETERTYPE_SRV;
	combinesig.mParameters[0].mResCounts = 1;
	combinesig.mParameters[0].mBindSlot = 0;
	combinesig.mParameters[0].mResource = &HDRBuffer;
	combinesig.mParameters[1].mType = PARAMETERTYPE_SRV;
	combinesig.mParameters[1].mResCounts = 1;
	combinesig.mParameters[1].mBindSlot = 1;
	combinesig.mParameters[1].mResource = &BloomBuffer;
	combinesig.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	combinesig.mParameters[2].mType = PARAMETERTYPE_SAMPLER;
	combinesig.mParameters[2].mResCounts = 1;
	combinesig.mParameters[2].mBindSlot = 0;
	combinesig.mParameters[2].mSampler = &gbuffersampler;
	combinesig.initialize(render.mDevice);


	ShaderSet combineshader;
	combineshader.shaders[VS].load("Shaders/CascadedShadowMaps/CombineDraw.hlsl", "VSMain", VS);
	combineshader.shaders[PS].load("Shaders/CascadedShadowMaps/CombineDraw.hlsl", "PSMain", PS);
	combinePipe.createGraphicsPipeline(render.mDevice, combinesig, combineshader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState());



	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.setBack(200);
	camera.updateViewProj();


	gamecamera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	gamecamera.setBack(300);
	gamecamera.updateViewProj();

}

void loadAsset()
{

	sunlight.setSliceNumber(shadowcount);
	sunlight.usesmsr(usesmsr);
	sunlight.setIntensity(sunintensity);
	float dirx = sin(spint*PI / 180.0f)*cos(tilt*PI / 180.0f);
	float diry = sin(spint*PI / 180.0f)*sin(tilt*PI / 180.0f);
	float dirz = cos(spint*PI / 180.0f);
	sunlight.setDirection(dirx, diry, dirz);



	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();







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


	meshList.resize(maxmeshcount);
	indirectMeshList.resize(maxmeshcount);
	materialList.resize(maxmeshcount);
	matricesList.resize(maxobjectcount);
	objectList.resize(maxobjectcount);


	Assimp::Importer sponzaimport;
	sponzaimport.ReadFile("Assets/sponza.obj", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);
	aiScene const * sponzascene = nullptr;
	sponzascene = sponzaimport.GetScene();

	// create all the structer buffers, conintuous registering 
	objectList.resize(sponzascene->mNumMeshes);
	objectBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(Object), objectList.size(), STRUCTERED_BUFFER_TYPE_READ);
	matricesBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(Matrices), matricesList.size(), STRUCTERED_BUFFER_TYPE_READ);
	indirecrtMeshBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectMeshData), indirectMeshList.size(), STRUCTERED_BUFFER_TYPE_READ);
	materialBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(Material), materialList.size(), STRUCTERED_BUFFER_TYPE_READ);

	// continuous create buffer and register in the heap
	indirectGeoCmdBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectDrawCmd), objectList.size(), STRUCTERED_BUFFER_TYPE_READ_WRITE, true);
	shadowIndirectCmdBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirecShadowDrawCmd), objectList.size(), STRUCTERED_BUFFER_TYPE_READ_WRITE, true);



	//create the first defualt texture, a 1*1 texture with value(0.5,0.5,1.0), which can use as defualt value for normal mapping that has no normal map
	textureList.resize(maxtexturecount);
	vector<int> diffuseindex;
	vector<int> linearindex;
	vector<int> normalindex;
	imageList.resize(maxtexturecount);


	DefualtTexture.CreateTexture(render,srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1);
	//DefualtTexture.addSahderResorceView(srvheap);
	//++texturecount;
	char defaultnormal[4];
	defaultnormal[0] = 127;
	defaultnormal[1] = 127;
	defaultnormal[2] = 255;
	defaultnormal[3] = 255;




	// read all texture file and register to srv heap continuously

	cout << "Number of Mateirlas" << sponzascene->mNumMaterials << endl;
	aiColor3D color;
	string Path = "Assets/Textures/";

	for (int i = 0; i < sponzascene->mNumMaterials; ++i)
	{

		Material mat;
		if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE)>0)
		{

			aiString tex;
			aiTextureMapMode mode = aiTextureMapMode_Wrap;
			sponzascene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &tex, nullptr, nullptr, nullptr, nullptr, &mode);
			//if (mode == aiTextureMapMode_Wrap)
			//	cout << "A Wrap Texture" << endl;
			//if (mode == aiTextureMapMode_Clamp)
			//	cout << "A Clamp Texture" << endl;
			//if (mode == aiTextureMapMode_Mirror)
			//	cout << "A Mirror Texture" << endl;
			//if (mode == aiTextureMapMode_Decal)
			//	cout << "A Decal Texture" << endl;
			//		cout << mode << endl;
			//sponzascene->mMaterials[i]->GetTexture
			//			cout << "Diffuse: " << tex.C_Str() << endl;
			string fullpath = Path + string(tex.C_Str());
			//			cout << fullpath << endl;
			imageList[texturecount].load(fullpath.c_str());
			textureList[texturecount].CreateTexture(render,srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, imageList[texturecount].mWidth, imageList[texturecount].mHeight,1,8);
		//	textureList[texturecount].addSahderResorceView(srvheap);
			mat.mChoose[MATERIALMAP_INDEX_COLOR] = 0.0;
			mat.mTextureIndex[MATERIALMAP_INDEX_COLOR] = texturecount + 1;// always +1 since 0 is default
			diffuseindex.push_back(texturecount);
			++texturecount;
			
		}
		else
		{
			sponzascene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			mat.mAlbedo.x = color[0];
			mat.mAlbedo.y = color[1];
			mat.mAlbedo.z = color[2];
			//		cout << color[0] << "," << color[1] << "," << color[2] << endl;
		}
		if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_NORMALS)>0)
		{

			aiString tex;
			sponzascene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &tex);
			//		cout << "Diffuse: " << tex.C_Str() << endl;
			string fullpath = Path + string(tex.C_Str());
			//		cout << fullpath << endl;
			imageList[texturecount].load(fullpath.c_str());
			textureList[texturecount].CreateTexture(render,srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, imageList[texturecount].mWidth, imageList[texturecount].mHeight,1,8);
		//	textureList[texturecount].addSahderResorceView(srvheap);
			mat.mChoose[MATERIALMAP_INDEX_NORMAL] = 1.0;
			mat.mTextureIndex[MATERIALMAP_INDEX_NORMAL] = texturecount + 1;// always +1 since 0 is default
			normalindex.push_back(texturecount);
			++texturecount;
		}


		if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_SHININESS)>0)
		{

			aiString tex;
			sponzascene->mMaterials[i]->GetTexture(aiTextureType_SHININESS, 0, &tex);
			//			cout << "Diffuse: " << tex.C_Str() << endl;
			string fullpath = Path + string(tex.C_Str());
			//			cout << fullpath << endl;
			imageList[texturecount].load(fullpath.c_str());
			textureList[texturecount].CreateTexture(render,srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, imageList[texturecount].mWidth, imageList[texturecount].mHeight,1,8);
		//	textureList[texturecount].addSahderResorceView(srvheap);
			mat.mChoose[MATERIALMAP_INDEX_ROUGHNESS] = 0.0;
			mat.mTextureIndex[MATERIALMAP_INDEX_ROUGHNESS] = texturecount + 1;// always +1 since 0 is default
			linearindex.push_back(texturecount);
			++texturecount;
		}


		if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_SPECULAR)>0)
		{

			aiString tex;
			sponzascene->mMaterials[i]->GetTexture(aiTextureType_SPECULAR, 0, &tex);
			//		cout << "Diffuse: " << tex.C_Str() << endl;
			string fullpath = Path + string(tex.C_Str());
			//			cout << fullpath << endl;
			imageList[texturecount].load(fullpath.c_str());
			textureList[texturecount].CreateTexture(render,srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, imageList[texturecount].mWidth, imageList[texturecount].mHeight,1,8);
	//		textureList[texturecount].addSahderResorceView(srvheap);
			mat.mChoose[MATERIALMAP_INDEX_METALIC] = 0.0;
			mat.mTextureIndex[MATERIALMAP_INDEX_METALIC] = texturecount + 1;// always +1 since 0 is default
			linearindex.push_back(texturecount);
			++texturecount;
		}


		materialList[i] = mat;
	}
	//sponzascene->
	Transform trans;
	trans.setScale(0.1, 0.1, 0.1);
	trans.setAngle(0, 90, 0);
	trans.CacNewTransform();
	matricesList[0] = trans.getMatrices();
	for (int i = 0; i < sponzascene->mNumMeshes; ++i)
	{
		meshList[i].loadMesh(sponzascene->mMeshes[i], render, cmdalloc, cmdlist);
		indirectMeshList[i] = meshList[i].getIndirectData();
		if (sponzascene->mMeshes[i]->mMaterialIndex == 0)
			cout << "Default Material" << endl;
		objectList[i].mMeshID = i;
		objectList[i].mMaterialID = sponzascene->mMeshes[i]->mMaterialIndex;
		objectList[i].mMatricesID = 0;

	}
	
	lightBuffer.createConstantBuffer(render.mDevice,srvheap,sizeof(DirectionLightData));
	lightBuffer.maptoCpu();
	


	gaussdata = curve.generateNormalizeWeight();


	gaussianconst.createConstantBuffer(render.mDevice, srvheap, sizeof(GaussionWeight));
	gaussianconst.maptoCpu();
	gaussianconst.updateBufferfromCpu(&gaussdata, sizeof(GaussionWeight));
	gaussianconst.unMaptoCpu();

	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(vertexBuffer , D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.resourceTransition(indirecrtMeshBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(materialBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(matricesBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(objectBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);


	cmdlist.updateBufferData(vertexBuffer, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(normalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));


	cmdlist.updateBufferData(indirecrtMeshBuffer, indirectMeshList.data(), indirectMeshList.size() * sizeof(IndirectMeshData));
	cmdlist.updateBufferData(materialBuffer, materialList.data(), materialList.size() * sizeof(Material));
	cmdlist.updateBufferData(matricesBuffer, matricesList.data(), matricesList.size() * sizeof(Matrices));
	cmdlist.updateBufferData(objectBuffer, objectList.data(), objectList.size() * sizeof(Object));


	cmdlist.resourceTransition(indirecrtMeshBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(materialBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(objectBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(matricesBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	

	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdlist.resourceTransition(shadowIndirectCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,true);

	

	// start to load image data to texture
	cmdlist.resourceTransition(DefualtTexture, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.updateTextureData(DefualtTexture, &defaultnormal);
	cmdlist.resourceTransition(DefualtTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,true);



	for (int i = 0; i < texturecount; ++i)
	{
		cmdlist.resourceTransition(textureList[i], D3D12_RESOURCE_STATE_COPY_DEST);
	}
	cmdlist.setBarrier();

	for (int i = 0; i < texturecount; ++i)
	{
		cmdlist.updateTextureData(textureList[i], imageList[i].mData,0,1);
	}
	for (int i = 0; i < texturecount; ++i)
	{
		cmdlist.resourceTransition(textureList[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	cmdlist.resourceTransition(GBuffer[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[2], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[3], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdlist.setBarrier();






	//cmdlist.renderTargetBarrier(HDRBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//	cmdlist.resourceBarrier(BloomBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//	cmdlist.resourceBarrier(BloomBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.insertSignalFence(fence);
	render.waitFence(fence);

	for (int i = 0; i < diffuseindex.size(); ++i)
	{
		render.generateMipMapOffline(textureList[diffuseindex[i]], MIP_MAP_GEN_SRGB_ALPHA_MASK_LINEAR_GAUSSIAN_CLAMP, 1);
	}

	for (int i = 0; i < linearindex.size(); ++i)
	{
		render.generateMipMapOffline(textureList[linearindex[i]], MIP_MAP_GEN_RGBA_LINEAR_GAUSSIAN_CLAMP, 1);
	}
	for (int i = 0; i < normalindex.size(); ++i)
	{
		render.generateMipMapOffline(textureList[normalindex[i]], MIP_MAP_GEN_RGBA_NORMAL_GAUSSIAN_CLAMP, 1);
	}
	//linearindex
	pre = std::chrono::high_resolution_clock::now();
}

void releaseRender()
{
	fence.release();
	depthDiscRootsig.realease();
	disconMap.release();

	combinePipe.release();
	combinesig.realease();

	gaussiansig.realease();
	brightextsig.realease();
	gaussianPipeline.release();
	brightExtractPipeline.release();
	//FinalBuffer.release();
	gaussianconst.release();
	TempBuffer.release();
	BloomBuffer.release();
	HDRBuffer.release();

	lightRootsig.realease();
	lightPipeline.release();
	depthDiscPipeline.release();

	shadowIndirectCmdBuffer.release();
	shadowCmdSig.release();

	shadowRootsig.realease();
	shadowPipeline.release();
	cascadedShadowMap.release();
	lightBuffer.release();

	GBuffer[3].release();
	GBuffer[2].release();
	GBuffer[1].release();
	GBuffer[0].release();
	rtvheap.release();
	dsvheap.release();
	samplerheap.release();
	for (int i = 0; i < texturecount; ++i)
		textureList[i].release();
	DefualtTexture.release();
	GeoDrawPipeline.release();
	geoDrawSig.realease();
	GeoCmdGenPipeline.release();
	indirecrtMeshBuffer.release();
	materialBuffer.release();
	matricesBuffer.release();
	objectBuffer.release();
	indirectGeoCmdBuffer.release();

	geoCmdGenRootSig.realease();
	geomCmdSig.release();
	for (int i = 0; i < maxmeshcount; ++i)
		meshList[i].release();

	import.FreeScene();
	cameraBuffer.release();
	indexBuffer.release();
	normalBuffer.release();
	srvheap.release();
	rootsig.realease();
	vertexBuffer.release();

	cmdlist.release();
	cmdalloc.release();
	render.releaseSwapChain();
	render.release();

}

void update()
{
	std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();



	std::chrono::duration<float> delta = t - pre;
	pre = t;




	//	cout << delta.count() << endl;

	gamecamera.updateViewProj();
	sunlight.updatewithCamera(gamecamera, shadowwidth, shadowheight);
	lightBuffer.updateBufferfromCpu(sunlight.getData(), sizeof(DirectionLightData));
	cameraBuffer.updateBufferfromCpu(gamecamera.getMatrix(), sizeof(ViewProjection));
	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(GeoCmdGenPipeline);
	cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);









	

	cmdlist.resourceTransition(indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_COPY_DEST); //reset count
	cmdlist.resourceTransition(shadowIndirectCmdBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.setCounterforStructeredBuffer(indirectGeoCmdBuffer, 0);
	cmdlist.setCounterforStructeredBuffer(shadowIndirectCmdBuffer, 0);
	cmdlist.resourceTransition(indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.resourceTransition(shadowIndirectCmdBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);

	cmdlist.bindComputeRootSigature(geoCmdGenRootSig);
	cmdlist.dispatch(objectList.size(), 1, 1);
	cmdlist.resourceTransition(indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdlist.resourceTransition(shadowIndirectCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);



	cmdlist.resourceTransition(GBuffer[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist.resourceTransition(GBuffer[2], D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist.resourceTransition(GBuffer[3], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(cascadedShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE,true);

	// shadowpass
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist.bindDepthStencilBufferOnly(cascadedShadowMap);
	cmdlist.clearDepthStencil(cascadedShadowMap);

	cmdlist.clearRenderTarget(GBuffer[0]);
	cmdlist.clearRenderTarget(GBuffer[1]);
	cmdlist.clearRenderTarget(GBuffer[2]);
	cmdlist.clearDepthStencil(GBuffer[3]);

	cmdlist.setViewPort(shadowviewport);
	cmdlist.setScissor(shadowscissor);
	cmdlist.bindPipeline(shadowPipeline);
	cmdlist.bindGraphicsRootSigature(shadowRootsig);
	cmdlist.executeIndirect(shadowCmdSig, objectList.size(), shadowIndirectCmdBuffer, 0,shadowIndirectCmdBuffer,shadowIndirectCmdBuffer.mBufferSize - sizeof(UINT));


	

	// G Pass
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	
	cmdlist.bindRenderTargetsDepthStencil(GBuffer[0],GBuffer[1], GBuffer[2], GBuffer[3]);





	cmdlist.bindPipeline(GeoDrawPipeline);
	cmdlist.bindGraphicsRootSigature(geoDrawSig);
	cmdlist.executeIndirect(geomCmdSig, objectList.size(), indirectGeoCmdBuffer, 0, indirectGeoCmdBuffer, indirectGeoCmdBuffer.mBufferSize - sizeof(UINT));

	cmdlist.resourceTransition(cascadedShadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[2], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[3], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


	//depht disc pass

	cmdlist.resourceTransition(disconMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);
	cmdlist.bindComputeRootSigature(depthDiscRootsig);
	cmdlist.bindPipeline(depthDiscPipeline);
	cmdlist.dispatch((disconMap.textureDesc.Width / 16) + 1, (disconMap.textureDesc.Height / 16) + 1, 1);
	cmdlist.resourceTransition(disconMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);






	// LightPass

	cmdlist.resourceTransition(HDRBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);
	cmdlist.bindComputeRootSigature(lightRootsig);
	cmdlist.bindPipeline(lightPipeline);
	cmdlist.dispatch((HDRBuffer.textureDesc.Width / 16) + 1, (HDRBuffer.textureDesc.Height / 16) + 1, 1);
	cmdlist.resourceTransition(HDRBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);



	//Post processing pass

	cmdlist.resourceTransition(BloomBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.resourceTransition(TempBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);
	cmdlist.bindComputeRootSigature(brightextsig);
	cmdlist.bindPipeline(brightExtractPipeline);
	cmdlist.bindComputeConstant(0, &bloomthreash);
	cmdlist.dispatch((BloomBuffer.textureDesc.Width / 256) + 1, BloomBuffer.textureDesc.Height, 1);

	cmdlist.resourceTransition(BloomBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,true);
	// gaussian blur use temp to store hor res

	//TempBuffer
	unsigned int direc = 0;
	unsigned int direc2 = 1;
	cmdlist.bindComputeRootSigature(gaussiansig);
	cmdlist.bindPipeline(gaussianPipeline);
	cmdlist.bindComputeConstant(1, &direc);
	cmdlist.bindComputeResource(2, BloomBuffer);
	cmdlist.bindComputeResource(3, TempBuffer);
	cmdlist.dispatch((BloomBuffer.textureDesc.Width / 256) + 1, BloomBuffer.textureDesc.Height, 1);

	//	cmdlist.dispatch(BloomBuffer.textureDesc.Width, (BloomBuffer.textureDesc.Height / 256) + 1, 1);

	cmdlist.resourceTransition(TempBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(BloomBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);
	//	direc = 1;
	cmdlist.bindComputeRootSigature(gaussiansig);
	cmdlist.bindPipeline(gaussianPipeline);
	cmdlist.bindComputeConstant(1, &direc2);
	cmdlist.bindComputeResource(2, TempBuffer);
	cmdlist.bindComputeResource(3, BloomBuffer);
	cmdlist.dispatch((BloomBuffer.textureDesc.Height / 256) + 1, BloomBuffer.textureDesc.Width, 1);




	cmdlist.resourceTransition(BloomBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex]);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdlist.bindPipeline(combinePipe);
	cmdlist.bindGraphicsRootSigature(combinesig);
	cmdlist.drawInstance(3, 1, 0, 0);


	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT,true);


	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.present();
	render.insertSignalFence(fence);
	render.waitFence(fence);
}


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	curxoffet = xpos - curxpos;
	curyoffet = ypos - curypos;

	curxpos = xpos;
	curypos = ypos;

	if (press&&!shiftpress)
		gamecamera.addAngle(curxoffet / 5.0, curyoffet / 5.0);
	if (press&&shiftpress)
	{
		tilt += curxoffet / 5.0;
		spint += curyoffet / 5.0;
		float dirx = sin(spint*PI / 180.0f)*cos(tilt*PI / 180.0f);
		float diry = sin(spint*PI / 180.0f)*sin(tilt*PI / 180.0f);
		float dirz = cos(spint*PI / 180.0f);
		//cout << spint << "   " << tilt << endl;
		sunlight.setDirection(dirx, diry, dirz);
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
		camera.addZoom(1);
	if (yoffset == -1)
		camera.addZoom(-1);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{


	if (key == GLFW_KEY_W)
		gamecamera.moveFrontBack(0.5f);
	if (key == GLFW_KEY_S)
		gamecamera.moveFrontBack(-0.5f);
	if (key == GLFW_KEY_D)
		gamecamera.moveLeftRight(0.5f);
	if (key == GLFW_KEY_A)
		gamecamera.moveLeftRight(-0.5f);
	if (key == GLFW_KEY_E)
		gamecamera.moveUpDown(0.5f);
	if (key == GLFW_KEY_Q)
		gamecamera.moveUpDown(-0.5f);

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		shiftpress = true;
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		shiftpress = false;

	if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
	{
		++shadowcount;
		shadowcount = std::min(shadowcount, 8u);
		sunlight.setSliceNumber(shadowcount);
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
	{
	
		--shadowcount;
		shadowcount = std::max(shadowcount, 1u);
		sunlight.setSliceNumber(shadowcount);
	}


	if (key == GLFW_KEY_1 && action == GLFW_RELEASE)
	{
		showcascaded =!showcascaded;
		sunlight.setshowcascadedcolor(showcascaded);
	}

	if (key == GLFW_KEY_2 && action == GLFW_RELEASE)
	{
		usesmsr = !usesmsr;
		sunlight.usesmsr(usesmsr);
	}




	if (key == GLFW_KEY_RIGHT)
	{
		sunintensity += 0.1f;
		sunlight.setIntensity(sunintensity);
	}
	if (key == GLFW_KEY_LEFT)
	{
		sunintensity -= 0.1f;
		sunintensity = max(0.0f, sunintensity);
		sunlight.setIntensity(sunintensity);
	}

}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "CascadedShadowMaps");
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