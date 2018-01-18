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
#include "PointLight.h"
#include "GaussionCurve.h"
#include <chrono>
#include <random>
#include <array>
using namespace std;
Render render;
CommandAllocator cmdalloc;
CommandList cmdlist;
Window windows;
Pipeline pipeline;
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
	unsigned int lightindexoffset;
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
std::vector<PointLightData> pointLightList;
// temp data for indirect piont light cmd
std::vector<IndirectPointLightDrawCmd> lightCmd;

Buffer indirecrtMeshBuffer;
Buffer materialBuffer;
Buffer matricesBuffer;
Buffer objectBuffer;
Buffer indirectGeoCmdBuffer;
Buffer pointLightBuffer;
Buffer pointLightIndirectBuffer;
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
vector<Texture> GBuffer;
vector<PointLight> pointLights;
vector<float> vel;
CommandSignature lightCmdSig;
//RootSignature geoCmdGenRootSig;
RootSignature lightDrawSig;
Pipeline lightPipeline;
Buffer shadowlightlistBuffer;
Buffer shadowIndirectCmdBuffer;
Texture cubeShadowMaps;
unsigned int shadowwidth = 256;
unsigned int shadowheight = 256;
RootSignature shadowRootSig;
CommandSignature shadowCmdSig;
Pipeline shadowpPipeline;
ViewPort shadowviewport;
Scissor shadowscissor;
Texture HDRBuffer;  // another positon why seperate textue and rendertarget is wrong, should use usage approach while creating resource
Texture BloomBuffer;
Texture TempBuffer;
#define GROUPSIZE 256
GaussionWeight gaussdata;

GaussionCurve curve(10, 3);
float bloomthreash = 1.0f;
RootSignature brightextsig;
RootSignature gaussiansig;
Buffer gaussianconst;
Sampler bilinear(D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT);
Pipeline brightExtractPipeline;
Pipeline gaussianPipeline;
Pipeline combinePipe;
RootSignature combinesig;
int pointLightNum = 128;

std::default_random_engine generator(3);
std::uniform_real_distribution<float> distributionXZ(-50.0, 50.0);
std::uniform_real_distribution<float> distributionY(5.0, 100.0f);
std::uniform_real_distribution<float> distributionintensity(100, 1500);
std::uniform_real_distribution<float> distributionradius(15.0, 20.0);;
std::array<double, 2> intervals{ 0.0, 1.0 };
std::array<float, 2> weights{ 1500 , 1500.0 };
std::piecewise_linear_distribution<double>
distributionlightcolor(intervals.begin(), intervals.end(), weights.begin());



std::array<double, 3> xintervals{-55, 0.0,55 };
std::array<float, 3> xweights{ 1000,1000,1000 };
std::piecewise_linear_distribution<double>
distributionX(xintervals.begin(), xintervals.end(), xweights.begin());


//std::uniform_real_distribution<float> distributionlightcolor(0.3, 1.5);
std::uniform_real_distribution<float> distributionmove(-10, 10);


// Thought: each objects is one basic draw call, each object don't have any information about material, matrics, mesh
// it only know the index in the list, 
//

// Defered pipeline
// First : use compute shader to generate frustum cull indirect draw call, since mesh has vertex and index buffer location, 
// can generate indirect draw call 
// input :  objectList that contains all objects in the scene
// cbv : camera buffer, use for culling, 
// output : frsutum culling indreict draw command, with a root constant, the root constant is the index of the object in the list

// Second : draw the visiblity scene indirect command in  G-Buffer
 
// Third : lit the scene with area lights

// Point light pipeline :
//  First : use compute shader to generate the indirect litting draw call that will lit the scene, 
//          also generate another cubeshadowmap indirect draw all for cube shadow mapping , need to cull out the objects that is not in
//          the range of point light
// Second : draw area volume or quad on the screen, should use very simple volume, like a box. Also, use cube shadow map to get nice shadow effect



/*******************************************************/
std::chrono::high_resolution_clock::time_point pre;
void initializeRender()
{


	//Camera cam;

	cout << sizeof(IndirecShadowDrawCmd) << endl;
//	system("pause");

	render.initialize();
	RenderTargetFormat retformat;
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
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
	//GBuffer.createRenderTargets(render.mDevice, windows.mWidth, windows.mHeight, gBufferFormat, rtvheap, dsvheap, srvheap);
	GBuffer.resize(4);
	GBuffer[0].CreateTexture(render, srvheap, gBufferFormat.mRenderTargetFormat[0], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);
	GBuffer[1].CreateTexture(render, srvheap, gBufferFormat.mRenderTargetFormat[1], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);
	GBuffer[2].CreateTexture(render, srvheap, gBufferFormat.mRenderTargetFormat[2], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);
	GBuffer[3].CreateTexture(render, srvheap, gBufferFormat.mDepthStencilFormat, windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_DSV);


	//DXGI_FORMAT hdrformat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	RenderTargetFormat hdrFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	//HDRBuffer.createRenderTargets(render.mDevice, windows.mWidth, windows.mHeight, hdrFormat, rtvheap, srvheap);
	//BloomBuffer.CreateTexture(render.mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, windows.mWidth/2, windows.mHeight/2, 1, false, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	//BloomBuffer.addUnorderedAccessView(srvheap);
	//BloomBuffer.addSahderResorceView(srvheap);


	//TempBuffer.CreateTexture(render.mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, windows.mWidth / 2, windows.mHeight / 2, 1, false, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	//TempBuffer.addUnorderedAccessView(srvheap);
	//TempBuffer.addSahderResorceView(srvheap);

	HDRBuffer.CreateTexture(render, srvheap, hdrFormat.mRenderTargetFormat[0], windows.mWidth, windows.mHeight, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);

	BloomBuffer.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, windows.mWidth / 2, windows.mHeight / 2, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);

	TempBuffer.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT, windows.mWidth / 2, windows.mHeight / 2, 1, 1, TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);

	//FinalBuffer.createRenderTargets(render.mDevice, windows.mWidth, windows.mHeight, retformat, rtvheap, srvheap);
	//FinalBuffer.CreateTexture(render.mDevice, DXGI_FORMAT_R8G8B8A8_UNORM, windows.mWidth, windows.mHeight, 1, false, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);


	samplerheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	matsampler.createSampler(samplerheap);
	gbuffersampler.createSampler(samplerheap);
	bilinear.createSampler(samplerheap);

	rootsig.mParameters.resize(1);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;



	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/ModelLoading.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/ModelLoading.hlsl", "PSMain", PS);


	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);



	geoCmdGenRootSig.mParameters.resize(4);
	geoCmdGenRootSig.mParameters[0].mType = PARAMETERTYPE_UAV; // indirect command + shadow light list + shadow command
	geoCmdGenRootSig.mParameters[0].mResCounts = 3;
	geoCmdGenRootSig.mParameters[0].mBindSlot = 0;
	geoCmdGenRootSig.mParameters[0].mResource = &indirectGeoCmdBuffer;
	geoCmdGenRootSig.mParameters[1].mType = PARAMETERTYPE_SRV; //objct + mac + mesh
	geoCmdGenRootSig.mParameters[1].mResCounts = 3;
	geoCmdGenRootSig.mParameters[1].mBindSlot = 0;
	geoCmdGenRootSig.mParameters[1].mResource = &objectBuffer;
	geoCmdGenRootSig.mParameters[2].mType = PARAMETERTYPE_SRV; 
	geoCmdGenRootSig.mParameters[2].mResCounts = 1;
	geoCmdGenRootSig.mParameters[2].mBindSlot = 3;
	geoCmdGenRootSig.mParameters[2].mResource = &pointLightBuffer;
	//geoCmdGenRootSig.mParameters[2].mType = PARAMETERTYPE_SRV;
	//geoCmdGenRootSig.mParameters[2].mResCounts = 1;
	//geoCmdGenRootSig.mParameters[2].mBindSlot = 1;
	//geoCmdGenRootSig.mParameters[2].mResource = &indirecrtMeshBuffer;
	//geoCmdGenRootSig.mParameters[3].mType = PARAMETERTYPE_SRV;
	//geoCmdGenRootSig.mParameters[3].mResCounts = 1;
	//geoCmdGenRootSig.mParameters[3].mBindSlot = 2;
	//geoCmdGenRootSig.mParameters[3].mResource = &matricesBuffer;
	geoCmdGenRootSig.mParameters[3].mType = PARAMETERTYPE_CBV;
	geoCmdGenRootSig.mParameters[3].mResCounts = 1;
	geoCmdGenRootSig.mParameters[3].mBindSlot = 0;
	geoCmdGenRootSig.mParameters[3].mResource = &cameraBuffer;


	geoCmdGenRootSig.initialize(render.mDevice);

	ShaderSet geocmdgencs;
	geocmdgencs.shaders[CS].load("Shaders/ObjectFrsutumCulling.hlsl", "CSMain", CS);
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
	geoDrawShader.shaders[VS].load("Shaders/MultipleShadowGeom.hlsl", "VSMain", VS);
//	geoDrawShader.shaders[GS].load("Shaders/MultipleShadowGeom.hlsl", "GSMain", GS);
	geoDrawShader.shaders[PS].load("Shaders/MultipleShadowGeom.hlsl", "PSMain", PS);

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


	shadowRootSig.mParameters.resize(5);
	shadowRootSig.mParameters[0].mType = PARAMETERTYPE_ROOTCONSTANT; // obj index + shaddow light index offset
	shadowRootSig.mParameters[0].mResCounts = 2;
	shadowRootSig.mParameters[0].mBindSlot = 0;
	shadowRootSig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	shadowRootSig.mParameters[1].mType = PARAMETERTYPE_SRV; // objct 
	shadowRootSig.mParameters[1].mResCounts = 1;
	shadowRootSig.mParameters[1].mBindSlot = 0;
	shadowRootSig.mParameters[1].mResource = &objectBuffer;
	shadowRootSig.mParameters[1].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	shadowRootSig.mParameters[2].mType = PARAMETERTYPE_SRV; // matrices
	shadowRootSig.mParameters[2].mResCounts = 1;
	shadowRootSig.mParameters[2].mBindSlot = 1;
	shadowRootSig.mParameters[2].mResource = &matricesBuffer;
	shadowRootSig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	shadowRootSig.mParameters[3].mType = PARAMETERTYPE_SRV; // shadow light list 
	shadowRootSig.mParameters[3].mResCounts = 1;
	shadowRootSig.mParameters[3].mBindSlot = 2;
	shadowRootSig.mParameters[3].mResource = &shadowlightlistBuffer;
	shadowRootSig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	shadowRootSig.mParameters[4].mType = PARAMETERTYPE_SRV; // point light buffer
	shadowRootSig.mParameters[4].mResCounts = 1;
	shadowRootSig.mParameters[4].mBindSlot = 3;
	shadowRootSig.mParameters[4].mResource = &pointLightBuffer;
	//shadowRootSig.mParameters[4].mVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
	shadowRootSig.initialize(render.mDevice);




	shadowCmdSig.mParameters.resize(4);
	shadowCmdSig.mParameters[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	shadowCmdSig.mParameters[0].VertexBuffer.Slot = 0;
	shadowCmdSig.mParameters[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
	shadowCmdSig.mParameters[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	shadowCmdSig.mParameters[2].Constant.Num32BitValuesToSet = 2; // first paramenter in root sig
	shadowCmdSig.mParameters[2].Constant.RootParameterIndex = 0;
	shadowCmdSig.mParameters[2].Constant.DestOffsetIn32BitValues = 0;
	shadowCmdSig.mParameters[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	shadowCmdSig.initialize(render.mDevice, shadowRootSig);


	ShaderSet ShadowShader;
	ShadowShader.shaders[VS].load("Shaders/MultiPointLightsCubeShadowMap.hlsl", "VSMain", VS);
	ShadowShader.shaders[GS].load("Shaders/MultiPointLightsCubeShadowMap.hlsl", "GSMain", GS);
	ShadowShader.shaders[PS].load("Shaders/MultiPointLightsCubeShadowMap.hlsl", "PSMain", PS);

	cubeShadowMaps.CreateTexture(render,srvheap, DXGI_FORMAT_R16_TYPELESS, shadowwidth, shadowheight, pointLightNum, 1,TEXTURE_SRV_TYPE_CUBE,TEXTURE_USAGE_SRV_DSV);
	RenderTargetFormat shadowformat(0, nullptr, true, true, DXGI_FORMAT_R16_TYPELESS);
	shadowpPipeline.createGraphicsPipeline(render.mDevice, shadowRootSig, ShadowShader, shadowformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(), VERTEX_LAYOUT_TYPE_SPLIT_ALL);
	//shadowpPipeline.createGraphicsPipeline()

	shadowviewport.setup(0, 0, (float)shadowwidth, (float)shadowheight);
	shadowscissor.setup(0, shadowwidth, 0, shadowheight);
	//Light pass 


	lightDrawSig.mParameters.resize(6);
	lightDrawSig.mParameters[0].mType = PARAMETERTYPE_ROOTCONSTANT;  // light index
	lightDrawSig.mParameters[0].mResCounts = 1;
	lightDrawSig.mParameters[0].mBindSlot = 1;
	lightDrawSig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	lightDrawSig.mParameters[1].mType = PARAMETERTYPE_CBV;  // camera buffer
	lightDrawSig.mParameters[1].mResCounts = 1;
	lightDrawSig.mParameters[1].mBindSlot = 0;
	lightDrawSig.mParameters[1].mResource = &cameraBuffer;
	lightDrawSig.mParameters[2].mType = PARAMETERTYPE_SRV;
	lightDrawSig.mParameters[2].mResCounts = 4;
	lightDrawSig.mParameters[2].mBindSlot = 0;
	lightDrawSig.mParameters[2].mResource = &GBuffer[0]; 
	lightDrawSig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	lightDrawSig.mParameters[3].mType = PARAMETERTYPE_SRV;
	lightDrawSig.mParameters[3].mResCounts = 1;
	lightDrawSig.mParameters[3].mBindSlot = 4;
	lightDrawSig.mParameters[3].mResource = &pointLightBuffer; 
	lightDrawSig.mParameters[4].mType = PARAMETERTYPE_SAMPLER;
	lightDrawSig.mParameters[4].mResCounts = 1;
	lightDrawSig.mParameters[4].mBindSlot = 0;
	lightDrawSig.mParameters[4].mSampler = &gbuffersampler;
	lightDrawSig.mParameters[4].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	lightDrawSig.mParameters[5].mType = PARAMETERTYPE_SRV;
	lightDrawSig.mParameters[5].mResCounts = 1;
	lightDrawSig.mParameters[5].mBindSlot = 5;
	lightDrawSig.mParameters[5].mResource = &cubeShadowMaps;
	lightDrawSig.mParameters[5].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	lightDrawSig.initialize(render.mDevice);

	ShaderSet lightshader;
	lightshader.shaders[VS].load("Shaders/MultiShadowPointLight.hlsl", "VSMain", VS);
	lightshader.shaders[GS].load("Shaders/MultiShadowPointLight.hlsl", "GSMain", GS);
	lightshader.shaders[PS].load("Shaders/MultiShadowPointLight.hlsl", "PSMain", PS);

	lightPipeline.createGraphicsPipeline(render.mDevice, lightDrawSig, lightshader, hdrFormat, DepthStencilState::DepthStencilState(), BlendState::BlendState(true), RasterizerState::RasterizerState(D3D12_CULL_MODE_NONE), VERTEX_LAYOUT_TYPE_SPLIT_ALL, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	 

	lightCmdSig.mParameters.resize(2);
	lightCmdSig.mParameters[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	lightCmdSig.mParameters[0].Constant.Num32BitValuesToSet = 1; // first paramenter in root sig
	lightCmdSig.mParameters[0].Constant.RootParameterIndex = 0;
	lightCmdSig.mParameters[0].Constant.DestOffsetIn32BitValues = 0;
	lightCmdSig.mParameters[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	lightCmdSig.initialize(render.mDevice, lightDrawSig);



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
	brightextsig.mParameters[3].mSampler = &bilinear;
	brightextsig.initialize(render.mDevice);


	// post process pass;
	ShaderSet brightshader;
	brightshader.shaders[CS].load("Shaders/BrightExtract.hlsl", "CSMain", CS);
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
	gaussianshader.shaders[CS].load("Shaders/GaussianBlur.hlsl", "CSMain", CS);
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
	combineshader.shaders[VS].load("Shaders/CombineDraw.hlsl", "VSMain", VS);
	combineshader.shaders[PS].load("Shaders/CombineDraw.hlsl", "PSMain", PS);
	combinePipe.createGraphicsPipeline(render.mDevice,combinesig,combineshader,retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(), RasterizerState::RasterizerState());
	


	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.setBack(2000);
	camera.updateViewProj();


	gamecamera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	gamecamera.setBack(2000);
	gamecamera.updateViewProj();

}

void loadAsset()
{



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
	sponzaimport.ReadFile("Assets/sponza.obj", aiProcess_CalcTangentSpace|aiProcess_Triangulate | aiProcess_FlipUVs);
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
	shadowlightlistBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(unsigned int), objectList.size()*pointLightNum+1, STRUCTERED_BUFFER_TYPE_READ_WRITE);
	shadowIndirectCmdBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirecShadowDrawCmd), objectList.size(), STRUCTERED_BUFFER_TYPE_READ_WRITE, true);
	// use the first one as a counter, but since countr doesn't offer add over 1 in one time, just need to use interlock

	// light data 
	vel.resize(pointLightNum);
	pointLights.resize(pointLightNum);
	pointLightList.resize(pointLightNum);
	pointLightBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(PointLightData), pointLightNum, STRUCTERED_BUFFER_TYPE_READ);
	PointLight light;
	//for(int i = 0 ; i < pointLightNum ; ++i)
	for (int i = 0; i < pointLightNum; ++i)
	{
		pointLights[i].setPosition(distributionX(generator), distributionY(generator), distributionXZ(generator));
		pointLights[i].setRadius(distributionradius(generator));
		pointLights[i].setColor(distributionlightcolor(generator), distributionlightcolor(generator), distributionlightcolor(generator));
		pointLights[i].setIntensity(distributionintensity(generator));
		vel[i] = distributionmove(generator);
		pointLightList[i] = *pointLights[i].getLightData();
	}


	//pointLights[0].setPosition(-35, 2, 0);
	//pointLights[0].setRadius(40);
	//pointLights[0].setColor(0.1, 0.1, 0.8);
	//pointLights[0].setIntensity(1000);
	//vel[0] = -10;
	//pointLightList[0] = *pointLights[0].getLightData();


	//pointLights[1].setPosition(0, 5, -35);
	//pointLights[1].setRadius(50);
	//pointLights[1].setIntensity(5000);
	//pointLights[1].setColor(0.8, 0.1, 0.0);
	//vel[1] = -10;
	//pointLightList[1] = *pointLights[1].getLightData();

	//pointLights[2].setPosition(-35, 65, -50);
	//pointLights[2].setRadius(30);
	//pointLights[2].setIntensity(15000);
	//pointLights[2].setColor(0.2, 0.6, 0.1);
	//vel[2] = 10;
	//pointLightList[2] = *pointLights[2].getLightData();


	//pointLights[3].setPosition( 3, 80, 0);
	//pointLights[3].setRadius(50);
	//pointLights[3].setIntensity(1000);
	//pointLights[3].setColor(0.2, 0.6, 0.6);
	//vel[3] = 0;
	//pointLightList[3] = *pointLights[3].getLightData();


	//pointLights[4].setPosition(35, 68, 0);
	//pointLights[4].setRadius(30);
	//pointLights[4].setIntensity(1000);
	//pointLights[4].setColor(0.7, 0.1, 0.6);
	//vel[4] = -10;
	//pointLightList[4] = *pointLights[4].getLightData();


	lightCmd.resize(pointLightNum);
	pointLightIndirectBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(IndirectPointLightDrawCmd), pointLightNum, STRUCTERED_BUFFER_TYPE_READ);
	for (int i = 0; i < pointLightNum; ++i)
	{
		lightCmd[i].lightindex = i;
		lightCmd[i].argument.VertexCountPerInstance = 1; // invoke one vertex
		lightCmd[i].argument.InstanceCount = 1;
		lightCmd[i].argument.StartVertexLocation = 0;
		lightCmd[i].argument.StartInstanceLocation = i;
	}


	

	//create the first defualt texture, a 1*1 texture with value(0.5,0.5,1.0), which can use as defualt value for normal mapping that has no normal map
	textureList.resize(maxtexturecount);
	imageList.resize(maxtexturecount);


	DefualtTexture.CreateTexture(render.mDevice, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1);
	DefualtTexture.addSahderResorceView(srvheap);
	//++texturecount;
	char defaultnormal[4];
	defaultnormal[0] = 127;
	defaultnormal[1] = 127;
	defaultnormal[2] = 255;
	defaultnormal[3] = 255;




	// read all texture file and register to srv heap continuously

	cout<<"Number of Mateirlas" << sponzascene->mNumMaterials << endl;
	aiColor3D color;
	string Path = "Assets/Textures/";

	for (int i = 0; i < sponzascene->mNumMaterials; ++i)
	{

		Material mat;
		if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE)>0)
		{
		
			aiString tex;
			aiTextureMapMode mode = aiTextureMapMode_Wrap;
			sponzascene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &tex,nullptr,nullptr,nullptr,nullptr,&mode);
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
			textureList[texturecount].CreateTexture(render,srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, imageList[texturecount].mWidth, imageList[texturecount].mHeight);
		//	textureList[texturecount].addSahderResorceView(srvheap);
			mat.mChoose[MATERIALMAP_INDEX_COLOR] = 0.0;
			mat.mTextureIndex[MATERIALMAP_INDEX_COLOR] = texturecount+1;// always +1 since 0 is default
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
			textureList[texturecount].CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, imageList[texturecount].mWidth, imageList[texturecount].mHeight);
		//	textureList[texturecount].addSahderResorceView(srvheap);
			mat.mChoose[MATERIALMAP_INDEX_NORMAL] = 1.0;
			mat.mTextureIndex[MATERIALMAP_INDEX_NORMAL] = texturecount + 1;// always +1 since 0 is default
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
			textureList[texturecount].CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, imageList[texturecount].mWidth, imageList[texturecount].mHeight);
		//	textureList[texturecount].addSahderResorceView(srvheap);
			mat.mChoose[MATERIALMAP_INDEX_ROUGHNESS] = 0.0;
			mat.mTextureIndex[MATERIALMAP_INDEX_ROUGHNESS] = texturecount + 1;// always +1 since 0 is default
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
			textureList[texturecount].CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, imageList[texturecount].mWidth, imageList[texturecount].mHeight);
		//	textureList[texturecount].addSahderResorceView(srvheap);
			mat.mChoose[MATERIALMAP_INDEX_METALIC] = 0.0;
			mat.mTextureIndex[MATERIALMAP_INDEX_METALIC] = texturecount + 1;// always +1 since 0 is default
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
		meshList[i].loadMesh(sponzascene->mMeshes[i],render,cmdalloc,cmdlist);
		indirectMeshList[i] = meshList[i].getIndirectData();
		if (sponzascene->mMeshes[i]->mMaterialIndex==0)
			cout << "Default Material" << endl;
		objectList[i].mMeshID = i;
		objectList[i].mMaterialID = sponzascene->mMeshes[i]->mMaterialIndex;
		objectList[i].mMatricesID = 0;
	
	}
	//meshList[0].loadMesh(sponzascene->mMeshes[0], render, cmdalloc, cmdlist);
	

	gaussdata = curve.generateNormalizeWeight();
	

	gaussianconst.createConstantBuffer(render.mDevice,srvheap,sizeof(GaussionWeight));
	gaussianconst.maptoCpu();
	gaussianconst.updateBufferfromCpu(&gaussdata, sizeof(GaussionWeight));
	gaussianconst.unMaptoCpu();

	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(indexBuffer,D3D12_RESOURCE_STATE_COPY_DEST);
	
	cmdlist.resourceTransition(indirecrtMeshBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(materialBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(matricesBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(objectBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(pointLightBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);



	


	cmdlist.updateBufferData(vertexBuffer, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(indexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(normalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));


	cmdlist.updateBufferData(indirecrtMeshBuffer, indirectMeshList.data(), indirectMeshList.size()*sizeof(IndirectMeshData));
	cmdlist.updateBufferData(materialBuffer, materialList.data(), materialList.size()*sizeof(Material));
	cmdlist.updateBufferData(matricesBuffer, matricesList.data(), matricesList.size()*sizeof(Matrices));
	cmdlist.updateBufferData(objectBuffer, objectList.data(), objectList.size()*sizeof(Object));
	cmdlist.updateBufferData(pointLightBuffer, pointLightList.data(), pointLightList.size() * sizeof(PointLightData));


	cmdlist.resourceTransition(indirecrtMeshBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(materialBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(objectBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(matricesBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(pointLightBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	cmdlist.resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdlist.resourceTransition(shadowIndirectCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);


	// start to load image data to texture
	cmdlist.resourceTransition(DefualtTexture, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.updateTextureData(DefualtTexture, &defaultnormal);
	cmdlist.resourceTransition(DefualtTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,true);

	for (int i = 0; i < texturecount; ++i)
		cmdlist.resourceTransition(textureList[i], D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.setBarrier();
	for (int i = 0; i < texturecount; ++i)
		cmdlist.updateTextureData(textureList[i], imageList[i].mData);

	for (int i = 0; i < texturecount; ++i)
		cmdlist.resourceTransition(textureList[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.setBarrier();


	cmdlist.resourceTransition(GBuffer[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[2], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[3], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );


	//temporoay update data from cpu, not sure whether should keep this 
	cmdlist.resourceTransition(pointLightIndirectBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.updateBufferData(pointLightIndirectBuffer, lightCmd.data(), lightCmd.size() * sizeof(IndirectPointLightDrawCmd));

	cmdlist.resourceTransition(pointLightIndirectBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	cmdlist.resourceTransition(shadowlightlistBuffer,  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	cmdlist.resourceTransition(cubeShadowMaps, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);


	cmdlist.resourceTransition(HDRBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,true);

//	cmdlist.resourceBarrier(BloomBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//	cmdlist.resourceBarrier(BloomBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.waitCommandsDone();
	pre = std::chrono::high_resolution_clock::now();
}

void releaseRender()
{
	
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

	shadowpPipeline.release();
	shadowCmdSig.release();
	shadowRootSig.realease();
	cubeShadowMaps.release();

	shadowIndirectCmdBuffer.release();
	shadowlightlistBuffer.release();

	lightCmdSig.release();
	lightPipeline.release();
	lightDrawSig.realease();
	pointLightIndirectBuffer.release();
	pointLightBuffer.release();

	GBuffer[3].release();
	GBuffer[2].release();
	GBuffer[1].release();
	GBuffer[0].release();
	rtvheap.release();
	dsvheap.release();
	samplerheap.release();
	for (int i = 0; i < textureList.size(); ++i)
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
	pipeline.release();
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



	float dir = -1;
	for (int i = 0; i < pointLightNum; ++i)
	{
		
		pointLights[i].addPosition(0, 0, vel[i]*delta.count());
		if (abs(pointLights[i].getLightData()->mPosition.z) > 120.0f)
			vel[i] *= -1;
		pointLightList[i] = *pointLights[i].getLightData();
	}

//	cout << delta.count() << endl;

	gamecamera.updateViewProj();
	cameraBuffer.updateBufferfromCpu(gamecamera.getMatrix(), sizeof(ViewProjection));
	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	cmdlist.reset(GeoCmdGenPipeline);
	cmdlist.bindDescriptorHeaps(&srvheap,&samplerheap);




	






	cmdlist.resourceTransition(pointLightBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_COPY_DEST); //reset count
	cmdlist.resourceTransition(shadowIndirectCmdBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.setCounterforStructeredBuffer(indirectGeoCmdBuffer, 0);
	cmdlist.setCounterforStructeredBuffer(shadowIndirectCmdBuffer, 0);
	cmdlist.updateBufferData(pointLightBuffer, pointLightList.data(), pointLightList.size() * sizeof(PointLightData));
	cmdlist.resourceTransition(indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.resourceTransition(shadowIndirectCmdBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.resourceTransition(pointLightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(shadowlightlistBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

	cmdlist.bindComputeRootSigature(geoCmdGenRootSig);
	cmdlist.dispatch(objectList.size() , 1, 1);
	cmdlist.resourceTransition(indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdlist.resourceTransition(shadowlightlistBuffer,D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(shadowIndirectCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	cmdlist.resourceTransition(cubeShadowMaps, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(GBuffer[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist.resourceTransition(GBuffer[2], D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist.resourceTransition(GBuffer[3], D3D12_RESOURCE_STATE_DEPTH_WRITE,true);
	// shadow pass
	cmdlist.bindPipeline(shadowpPipeline);
	cmdlist.bindGraphicsRootSigature(shadowRootSig);
	cmdlist.setViewPort(shadowviewport);
	cmdlist.setScissor(shadowscissor);

	cmdlist.bindDepthStencilBufferOnly(cubeShadowMaps);
	cmdlist.clearDepthStencil(cubeShadowMaps);

	cmdlist.executeIndirect(shadowCmdSig, objectList.size(), shadowIndirectCmdBuffer, 0, shadowIndirectCmdBuffer, shadowIndirectCmdBuffer.mBufferSize - sizeof(UINT));


	
	// G Pass
	cmdlist.bindPipeline(pipeline);
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	



	cmdlist.bindRenderTargetsDepthStencil(GBuffer[0], GBuffer[1], GBuffer[2], GBuffer[3]);
	cmdlist.clearRenderTarget(GBuffer[0]);
	cmdlist.clearRenderTarget(GBuffer[1]);
	cmdlist.clearRenderTarget(GBuffer[2]);
	cmdlist.clearDepthStencil(GBuffer[3]);




	cmdlist.bindPipeline(GeoDrawPipeline);
	cmdlist.bindGraphicsRootSigature(geoDrawSig);
	cmdlist.executeIndirect(geomCmdSig, objectList.size(), indirectGeoCmdBuffer, 0, indirectGeoCmdBuffer, indirectGeoCmdBuffer.mBufferSize - sizeof(UINT));
	cmdlist.resourceTransition(GBuffer[0],D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[2], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(GBuffer[3], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdlist.resourceTransition(cubeShadowMaps, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);






	// LightPass
	
	cmdlist.resourceTransition(HDRBuffer,D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	//cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex]);
	//const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	//cmdlist.clearDepthStencil(render.mSwapChainRenderTarget[frameIndex]);

	cmdlist.bindRenderTargetsOnly(HDRBuffer);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdlist.clearRenderTarget(HDRBuffer);




	cmdlist.bindPipeline(lightPipeline);
	cmdlist.bindGraphicsRootSigature(lightDrawSig);
	/*int i = 0;
	cmdlist.bindGraphicsConstant(0, &i);
	cmdlist.drawInstance(3, 1, 0, 0);*/


	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	cmdlist.executeIndirect(lightCmdSig, pointLightNum, pointLightIndirectBuffer, 0);
	cmdlist.resourceTransition(HDRBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//Post processing pass

	cmdlist.resourceTransition(BloomBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cmdlist.resourceTransition(TempBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,true);
	cmdlist.bindComputeRootSigature(brightextsig);
	cmdlist.bindPipeline(brightExtractPipeline);
	cmdlist.bindComputeConstant(0, &bloomthreash);
	cmdlist.dispatch((BloomBuffer.textureDesc.Width / 256)+1, BloomBuffer.textureDesc.Height, 1);

	cmdlist.resourceTransition(BloomBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,true);
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

	cmdlist.resourceTransition(TempBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
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

	render.waitCommandsDone();
}


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	curxoffet = xpos - curxpos;
	curyoffet = ypos - curypos;

	curxpos = xpos;
	curypos = ypos;

	if (press)
		gamecamera.addAngle(curxoffet / 5.0, curyoffet / 5.0);
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


}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "MultiplePointLightShadow");
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