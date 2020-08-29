#include "GameCamera.h"
#include "GaussionCurve.h"
#include "Object.h"
#include "PointLight.h"
#include "Render.h"
#include "SpecCamera.h"
#include "StructureHeaders.h"
#include "Window.h"
#include "imgui/imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_glfw.h"
#include <algorithm>
#include <array>
#include <assimp\Importer.hpp>
#include <assimp\ai_assert.h>
#include <assimp\mesh.h>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>
using namespace std;
Render render;
static uint32_t swapChainCount = 3;
CommandAllocator cmdalloc[3];
CommandList cmdlist[3];
Fence fences[3];
Window windows;
UINT windowWidth = 1600;
UINT windowHeight = 900;
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
DescriptorHeap imgui_srvheap;
ShaderSet shaderset;
Assimp::Importer import;
SpecCamera camera;
GameCamera gamecamera;
aiScene const *scene = nullptr;
aiMesh *mesh = nullptr;
float curxpos;
float curypos;
float curxoffet;
float curyoffet;
bool press = false;

struct IndirectDrawCmd {
  VertexBufferView position;
  VertexBufferView uv;
  VertexBufferView normal;
  VertexBufferView tangent;
  VertexBufferView bitangent;
  IndexBufferView index;
  unsigned int objectindex;
  DrawIndexedArgument argument;
};
CommandSignature geomCmdSig;
RootSignature geoCmdGenRootSig;
// Prototype of dynmaic index with exectue indirect pipeline
std::vector<Mesh> meshList;
std::vector<IndirectMeshData> indirectMeshList;
std::vector<Material> materialList;
std::vector<Matrices> matricesList;
std::vector<Object> objectList;
std::vector<PointLightData> pointLightList;

Buffer indirecrtMeshBuffer;
Buffer materialBuffer;
Buffer matricesBuffer;
Buffer objectBuffer;
Buffer indirectGeoCmdBuffer;
Buffer pointLightBuffer;
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
bool moveLight = true;
bool debugDraw = false;
DescriptorHeap samplerheap;
Sampler matsampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                   D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                   D3D12_TEXTURE_ADDRESS_MODE_WRAP);
Sampler gbuffersampler;
vector<PointLight> pointLights;
vector<float> vel;
UINT objectCullingSize = 256;
Texture HDRBuffer; // another positon why seperate textue and rendertarget is
                   // wrong, should use usage approach while creating resource
Texture BloomBuffer;
Texture TempBuffer;
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
UINT pointLightNum = 1024;
int visibleLightNum = pointLightNum;

std::default_random_engine generator(3);
std::uniform_real_distribution<float> distributionXZ(-110.0, 110.0);
std::uniform_real_distribution<float> distributionY(5.0, 70.0f);
std::uniform_real_distribution<float> distributionintensity(500, 1000);
std::uniform_real_distribution<float> distributionradius(5.0, 40.0);
std::array<double, 2> intervals{0.0, 1.0};
std::array<float, 2> weights{1500, 1500.0};
std::piecewise_linear_distribution<double>
    distributionlightcolor(intervals.begin(), intervals.end(), weights.begin());

std::array<double, 3> xintervals{-55, 0.0, 55};
std::array<float, 3> xweights{1000, 1000, 1000};
std::piecewise_linear_distribution<double>
    distributionX(xintervals.begin(), xintervals.end(), xweights.begin());

std::uniform_real_distribution<float> distributionmove(-10, 10);
DynamicUploadBuffer uploadbuffer;
UINT64 framenumber = 1;
struct SceneData {
  UINT pointLightNum;
  UINT frameWidth;
  UINT frameHeight;
  bool debugDraw;
};
SceneData sceneData;

UINT blockWidth = 16;
UINT blockHeight = 16;
UINT blockWidthCount = ((windowWidth + blockWidth - 1) / blockWidth);
UINT blockHeightCount = ((windowHeight + blockHeight - 1) / blockHeight);
Buffer pointLightIndicesBuffer;
Buffer blockLightCountBuffer;
Buffer scenedataBuffer;
Pipeline lightCullingPipe;
RootSignature lightCullingSig;
Pipeline fowardPlusPipe;
RootSignature fowardPlusSig;
CommandSignature fowardCmdSig;
Texture DepthTexture;
std::chrono::high_resolution_clock::time_point pre;
void initializeRender() {

  sceneData.pointLightNum = visibleLightNum;
  sceneData.frameWidth = windowWidth;
  sceneData.frameHeight = windowHeight;
  sceneData.debugDraw = false;
  render.initialize();
  RenderTargetFormat retformat;
  render.createSwapChain(windows, swapChainCount,
                         retformat.mRenderTargetFormat[0]);
  for (int i = 0; i < swapChainCount; ++i) {
    cmdalloc[i].initialize(render.mDevice);
    cmdlist[i].initial(render.mDevice, cmdalloc[i]);
    fences[i].initialize(render);
  }

  uploadbuffer.initialize(render);

  srvheap.ininitialize(render.mDevice, 1);
  rtvheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  dsvheap.ininitialize(render.mDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  imgui_srvheap.ininitialize(render.mDevice, 100);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(windows.mWindow, true);
  ImGui_ImplDX12_Init(render.mDevice, swapChainCount,
                      retformat.mRenderTargetFormat[0],
                      imgui_srvheap.getHandles(SRV, 0).Cpu,
                      imgui_srvheap.getHandles(SRV, 0).Gpu);

  RenderTargetFormat depthformat(0, nullptr, true);
  DepthTexture.CreateTexture(render, srvheap, depthformat.mDepthStencilFormat,
                             windows.mWidth, windows.mHeight, 1, 1,
                             TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_DSV);

  RenderTargetFormat hdrFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);

  HDRBuffer.CreateTexture(render, srvheap, hdrFormat.mRenderTargetFormat[0],
                          windows.mWidth, windows.mHeight, 1, 1,
                          TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_RTV);

  BloomBuffer.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT,
                            windows.mWidth / 2, windows.mHeight / 2, 1, 1,
                            TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);

  TempBuffer.CreateTexture(render, srvheap, DXGI_FORMAT_R32G32B32A32_FLOAT,
                           windows.mWidth / 2, windows.mHeight / 2, 1, 1,
                           TEXTURE_SRV_TYPE_2D, TEXTURE_USAGE_SRV_UAV);

  samplerheap.ininitialize(render.mDevice, 1,
                           D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  matsampler.createSampler(samplerheap);
  gbuffersampler.createSampler(samplerheap);
  bilinear.createSampler(samplerheap);

  rootsig.mParameters.resize(1);
  rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
  rootsig.mParameters[0].mResCounts = 1;
  rootsig.mParameters[0].mBindSlot = 0;
  rootsig.mParameters[0].mResource = &cameraBuffer;

  rootsig.initialize(render.mDevice);

  geoCmdGenRootSig.mParameters.resize(3);
  geoCmdGenRootSig.mParameters[0].mType =
      PARAMETERTYPE_UAV; // indirect command + shadow light list + shadow
                         // command
  geoCmdGenRootSig.mParameters[0].mResCounts = 1;
  geoCmdGenRootSig.mParameters[0].mBindSlot = 0;
  geoCmdGenRootSig.mParameters[0].mResource = &indirectGeoCmdBuffer;
  geoCmdGenRootSig.mParameters[1].mType =
      PARAMETERTYPE_SRV; // objct + mac + mesh
  geoCmdGenRootSig.mParameters[1].mResCounts = 3;
  geoCmdGenRootSig.mParameters[1].mBindSlot = 0;
  geoCmdGenRootSig.mParameters[1].mResource = &objectBuffer;
  geoCmdGenRootSig.mParameters[2].mType = PARAMETERTYPE_CBV;
  geoCmdGenRootSig.mParameters[2].mResCounts = 1;
  geoCmdGenRootSig.mParameters[2].mBindSlot = 0;
  geoCmdGenRootSig.mParameters[2].mResource = &cameraBuffer;

  geoCmdGenRootSig.initialize(render.mDevice);

  ShaderSet geocmdgencs;
  geocmdgencs.shaders[CS].load("Shaders/FowardPlus/ObjectFrsutumCulling.hlsl",
                               "CSMain", CS);
  GeoCmdGenPipeline.createComputePipeline(render.mDevice, geoCmdGenRootSig,
                                          geocmdgencs);

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
  geoDrawSig.mParameters[6].rangeflag =
      D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
  geoDrawSig.mParameters[6].mResource = &DefualtTexture;
  geoDrawSig.mParameters[6].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  geoDrawSig.initialize(render.mDevice);

  ShaderSet geoDrawShader;
  geoDrawShader.shaders[VS].load("Shaders/FowardPlus/Depth.hlsl", "VSMain", VS);
  geoDrawShader.shaders[PS].load("Shaders/FowardPlus/Depth.hlsl", "PSMain", PS);
  GeoDrawPipeline.createGraphicsPipeline(
      render.mDevice, geoDrawSig, geoDrawShader, depthformat,
      DepthStencilState::DepthStencilState(true), BlendState::BlendState(),
      RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT),
      VERTEX_LAYOUT_TYPE_SPLIT_ALL);

  // fowardPlusSig;

  fowardPlusSig.mParameters.resize(11);
  fowardPlusSig.mParameters[0].mType =
      PARAMETERTYPE_ROOTCONSTANT; // object index
  fowardPlusSig.mParameters[0].mResCounts = 1;
  fowardPlusSig.mParameters[0].mBindSlot = 1;
  fowardPlusSig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  fowardPlusSig.mParameters[1].mType = PARAMETERTYPE_SRV; // objct
  fowardPlusSig.mParameters[1].mResCounts = 1;
  fowardPlusSig.mParameters[1].mBindSlot = 0;
  fowardPlusSig.mParameters[1].mResource = &objectBuffer;
  fowardPlusSig.mParameters[2].mType = PARAMETERTYPE_SRV; // matrices
  fowardPlusSig.mParameters[2].mResCounts = 1;
  fowardPlusSig.mParameters[2].mBindSlot = 1;
  fowardPlusSig.mParameters[2].mResource = &matricesBuffer;
  fowardPlusSig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  fowardPlusSig.mParameters[3].mType = PARAMETERTYPE_SRV; // material
  fowardPlusSig.mParameters[3].mResCounts = 1;
  fowardPlusSig.mParameters[3].mBindSlot = 2;
  fowardPlusSig.mParameters[3].mResource = &materialBuffer;
  fowardPlusSig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  fowardPlusSig.mParameters[4].mType = PARAMETERTYPE_CBV;
  fowardPlusSig.mParameters[4].mResCounts = 1;
  fowardPlusSig.mParameters[4].mBindSlot = 0;
  fowardPlusSig.mParameters[4].mResource = &cameraBuffer;
  fowardPlusSig.mParameters[4].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
  fowardPlusSig.mParameters[5].mType = PARAMETERTYPE_SAMPLER;
  fowardPlusSig.mParameters[5].mResCounts = 1;
  fowardPlusSig.mParameters[5].mBindSlot = 0;
  fowardPlusSig.mParameters[5].mSampler = &matsampler;
  fowardPlusSig.mParameters[5].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  fowardPlusSig.mParameters[6].mType = PARAMETERTYPE_SRV;
  fowardPlusSig.mParameters[6].mResCounts = -1;
  fowardPlusSig.mParameters[6].mBindSlot = 0;
  fowardPlusSig.mParameters[6].mShaderSpace = 1;
  fowardPlusSig.mParameters[6].rangeflag =
      D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
  fowardPlusSig.mParameters[6].mResource = &DefualtTexture;
  fowardPlusSig.mParameters[6].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  fowardPlusSig.mParameters[7].mType = PARAMETERTYPE_SRV;
  fowardPlusSig.mParameters[7].mResCounts = 1;
  fowardPlusSig.mParameters[7].mBindSlot = 3;
  fowardPlusSig.mParameters[7].mResource = &pointLightIndicesBuffer;
  fowardPlusSig.mParameters[7].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  fowardPlusSig.mParameters[8].mType = PARAMETERTYPE_SRV;
  fowardPlusSig.mParameters[8].mResCounts = 1;
  fowardPlusSig.mParameters[8].mBindSlot = 4;
  fowardPlusSig.mParameters[8].mResource = &blockLightCountBuffer;
  fowardPlusSig.mParameters[8].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  fowardPlusSig.mParameters[9].mType = PARAMETERTYPE_SRV;
  fowardPlusSig.mParameters[9].mResCounts = 1;
  fowardPlusSig.mParameters[9].mBindSlot = 5;
  fowardPlusSig.mParameters[9].mResource = &scenedataBuffer;
  fowardPlusSig.mParameters[9].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  fowardPlusSig.mParameters[10].mType = PARAMETERTYPE_SRV;
  fowardPlusSig.mParameters[10].mResCounts = 1;
  fowardPlusSig.mParameters[10].mBindSlot = 6;
  fowardPlusSig.mParameters[10].mResource = &pointLightBuffer;
  fowardPlusSig.mParameters[10].mVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  fowardPlusSig.initialize(render.mDevice);

  ShaderSet fowardShader;

  fowardShader.shaders[VS].load("Shaders/FowardPlus/FowardPlus.hlsl", "VSMain",
                                VS);
  fowardShader.shaders[PS].load("Shaders/FowardPlus/FowardPlus.hlsl", "PSMain",
                                PS);

  fowardPlusPipe.createGraphicsPipeline(
      render.mDevice, fowardPlusSig, fowardShader, hdrFormat,
      DepthStencilState::DepthStencilState(true, false),
      BlendState::BlendState(),
      RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT),
      VERTEX_LAYOUT_TYPE_SPLIT_ALL);

  geomCmdSig.mParameters.resize(8);
  geomCmdSig.mParameters[0].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  geomCmdSig.mParameters[0].VertexBuffer.Slot = 0;
  geomCmdSig.mParameters[1].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  geomCmdSig.mParameters[1].VertexBuffer.Slot = 1;
  geomCmdSig.mParameters[2].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  geomCmdSig.mParameters[2].VertexBuffer.Slot = 2;
  geomCmdSig.mParameters[3].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  geomCmdSig.mParameters[3].VertexBuffer.Slot = 3;
  geomCmdSig.mParameters[4].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  geomCmdSig.mParameters[4].VertexBuffer.Slot = 4;
  geomCmdSig.mParameters[5].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
  geomCmdSig.mParameters[6].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
  geomCmdSig.mParameters[6].Constant.Num32BitValuesToSet =
      1; // first paramenter in root sig
  geomCmdSig.mParameters[6].Constant.RootParameterIndex = 0;
  geomCmdSig.mParameters[6].Constant.DestOffsetIn32BitValues = 0;
  geomCmdSig.mParameters[7].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
  geomCmdSig.initialize(render.mDevice, geoDrawSig);

  fowardCmdSig.mParameters.resize(8);
  fowardCmdSig.mParameters[0].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  fowardCmdSig.mParameters[0].VertexBuffer.Slot = 0;
  fowardCmdSig.mParameters[1].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  fowardCmdSig.mParameters[1].VertexBuffer.Slot = 1;
  fowardCmdSig.mParameters[2].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  fowardCmdSig.mParameters[2].VertexBuffer.Slot = 2;
  fowardCmdSig.mParameters[3].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  fowardCmdSig.mParameters[3].VertexBuffer.Slot = 3;
  fowardCmdSig.mParameters[4].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  fowardCmdSig.mParameters[4].VertexBuffer.Slot = 4;
  fowardCmdSig.mParameters[5].Type =
      D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
  fowardCmdSig.mParameters[6].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
  fowardCmdSig.mParameters[6].Constant.Num32BitValuesToSet =
      1; // first paramenter in root sig
  fowardCmdSig.mParameters[6].Constant.RootParameterIndex = 0;
  fowardCmdSig.mParameters[6].Constant.DestOffsetIn32BitValues = 0;
  fowardCmdSig.mParameters[7].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
  fowardCmdSig.initialize(render.mDevice, fowardPlusSig);

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
  brightshader.shaders[CS].load("Shaders/FowardPlus/BrightExtract.hlsl",
                                "CSMain", CS);
  brightExtractPipeline.createComputePipeline(render.mDevice, brightextsig,
                                              brightshader);

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
  gaussianshader.shaders[CS].load("Shaders/FowardPlus/GaussianBlur.hlsl",
                                  "CSMain", CS);
  gaussianPipeline.createComputePipeline(render.mDevice, gaussiansig,
                                         gaussianshader);

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
  combineshader.shaders[VS].load("Shaders/FowardPlus/CombineDraw.hlsl",
                                 "VSMain", VS);
  combineshader.shaders[PS].load("Shaders/FowardPlus/CombineDraw.hlsl",
                                 "PSMain", PS);
  combinePipe.createGraphicsPipeline(
      render.mDevice, combinesig, combineshader, retformat,
      DepthStencilState::DepthStencilState(), BlendState::BlendState(),
      RasterizerState::RasterizerState());

  pointLightIndicesBuffer.createStructeredBuffer(
      render.mDevice, srvheap, sizeof(UINT),
      (pointLightNum * blockWidthCount * blockHeightCount + 1),
      STRUCTERED_BUFFER_TYPE_READ_WRITE);

  // each elemnt is (start,count)
  blockLightCountBuffer.createStructeredBuffer(
      render.mDevice, srvheap, sizeof(UINT),
      blockWidthCount * blockHeightCount * 2,
      STRUCTERED_BUFFER_TYPE_READ_WRITE);

  scenedataBuffer.createStructeredBuffer(render.mDevice, srvheap,
                                         sizeof(SceneData), 1,
                                         STRUCTERED_BUFFER_TYPE_READ_WRITE);

  lightCullingSig.mParameters.resize(6);
  lightCullingSig.mParameters[0].mType = PARAMETERTYPE_SRV;
  lightCullingSig.mParameters[0].mResCounts = 1;
  lightCullingSig.mParameters[0].mBindSlot = 0;
  lightCullingSig.mParameters[0].mResource = &pointLightBuffer;
  lightCullingSig.mParameters[1].mType = PARAMETERTYPE_UAV;
  lightCullingSig.mParameters[1].mResCounts = 1;
  lightCullingSig.mParameters[1].mBindSlot = 0;
  lightCullingSig.mParameters[1].mResource = &pointLightIndicesBuffer;
  lightCullingSig.mParameters[2].mType = PARAMETERTYPE_UAV;
  lightCullingSig.mParameters[2].mResCounts = 1;
  lightCullingSig.mParameters[2].mBindSlot = 1;
  lightCullingSig.mParameters[2].mResource = &blockLightCountBuffer;
  lightCullingSig.mParameters[3].mType = PARAMETERTYPE_CBV;
  lightCullingSig.mParameters[3].mResCounts = 1;
  lightCullingSig.mParameters[3].mBindSlot = 0;
  lightCullingSig.mParameters[3].mResource = &cameraBuffer;
  lightCullingSig.mParameters[4].mType = PARAMETERTYPE_SRV;
  lightCullingSig.mParameters[4].mResCounts = 1;
  lightCullingSig.mParameters[4].mBindSlot = 1;
  lightCullingSig.mParameters[4].mResource = &DepthTexture;
  lightCullingSig.mParameters[5].mType = PARAMETERTYPE_SRV;
  lightCullingSig.mParameters[5].mResCounts = 1;
  lightCullingSig.mParameters[5].mBindSlot = 2;
  lightCullingSig.mParameters[5].mResource = &scenedataBuffer;

  lightCullingSig.initialize(render.mDevice);

  ShaderSet lightcullshader;
  lightcullshader.shaders[CS].load("Shaders/FowardPlus/LightCulling.hlsl",
                                   "CSMain", CS);

  lightCullingPipe.createComputePipeline(render.mDevice, lightCullingSig,
                                         lightcullshader);

  viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
  scissor.setup(0, windows.mWidth, 0, windows.mHeight);

  camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
  camera.setBack(2000);
  camera.updateViewProj();

  gamecamera.setRatio((float)windows.mWidth / (float)windows.mHeight);
  gamecamera.setBack(2000);
  gamecamera.updateViewProj();
}

void loadAsset() {
  cameraBuffer.createConstanBufferNew(render, srvheap, sizeof(ViewProjection));

  import.ReadFile("Assets/sphere.obj", aiProcessPreset_TargetRealtime_Quality);
  scene = import.GetScene();
  mesh = scene->mMeshes[0];
  mesh->HasPositions();
  UINT verticnum = mesh->mNumVertices;

  if (!vertexBuffer.createVertexBuffer(render.mDevice,
                                       mesh->mNumVertices * 3 * sizeof(float),
                                       3 * sizeof(float))) {
    cout << "Fail for create" << endl;
  }
  normalBuffer.createVertexBuffer(render.mDevice,
                                  mesh->mNumVertices * 3 * sizeof(float),
                                  3 * sizeof(float));

  indexBuffer.createIndexBuffer(render.mDevice,
                                sizeof(unsigned int) * 3 * mesh->mNumFaces);

  std::vector<unsigned int> indexdata;
  indexdata.resize(mesh->mNumFaces * 3);
  for (int i = 0; i < mesh->mNumFaces; i++) {
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
  sponzaimport.ReadFile("Assets/sponza.obj", aiProcess_CalcTangentSpace |
                                                 aiProcess_Triangulate |
                                                 aiProcess_FlipUVs);
  aiScene const *sponzascene = nullptr;
  sponzascene = sponzaimport.GetScene();

  // create all the structer buffers, conintuous registering
  objectList.resize(sponzascene->mNumMeshes);
  objectBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(Object),
                                      objectList.size(),
                                      STRUCTERED_BUFFER_TYPE_READ);
  matricesBuffer.createStructeredBuffer(render.mDevice, srvheap,
                                        sizeof(Matrices), matricesList.size(),
                                        STRUCTERED_BUFFER_TYPE_READ);
  indirecrtMeshBuffer.createStructeredBuffer(
      render.mDevice, srvheap, sizeof(IndirectMeshData),
      indirectMeshList.size(), STRUCTERED_BUFFER_TYPE_READ);
  materialBuffer.createStructeredBuffer(render.mDevice, srvheap,
                                        sizeof(Material), materialList.size(),
                                        STRUCTERED_BUFFER_TYPE_READ);

  // continuous create buffer and register in the heap
  indirectGeoCmdBuffer.createStructeredBuffer(
      render.mDevice, srvheap, sizeof(IndirectDrawCmd), objectList.size(),
      STRUCTERED_BUFFER_TYPE_READ_WRITE, true);

  // light data
  vel.resize(pointLightNum);
  pointLights.resize(pointLightNum);
  pointLightList.resize(pointLightNum);
  pointLightBuffer.createStructeredBuffer(render.mDevice, srvheap,
                                          sizeof(PointLightData), pointLightNum,
                                          STRUCTERED_BUFFER_TYPE_READ);
  PointLight light;
  // for(int i = 0 ; i < pointLightNum ; ++i)
  for (int i = 0; i < pointLightNum; ++i) {
    pointLights[i].setPosition(distributionX(generator),
                               distributionY(generator),
                               distributionXZ(generator));
    pointLights[i].setRadius(distributionradius(generator));
    pointLights[i].setColor(distributionlightcolor(generator),
                            distributionlightcolor(generator),
                            distributionlightcolor(generator));
    pointLights[i].setIntensity(distributionintensity(generator));
    vel[i] = distributionmove(generator);
    pointLightList[i] = *pointLights[i].getLightData();
  }

  textureList.resize(maxtexturecount);
  imageList.resize(maxtexturecount);

  DefualtTexture.CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, 1,
                               1);
  // DefualtTexture.addSahderResorceView(srvheap);
  //++texturecount;
  char defaultnormal[4];
  defaultnormal[0] = 127;
  defaultnormal[1] = 127;
  defaultnormal[2] = 255;
  defaultnormal[3] = 255;

  vector<int> diffuseindex;
  vector<int> linearindex;
  vector<int> normalindex;

  // read all texture file and register to srv heap continuously

  cout << "Number of Mateirlas" << sponzascene->mNumMaterials << endl;
  aiColor3D color;
  string Path = "Assets/Textures/";

  for (int i = 0; i < sponzascene->mNumMaterials; ++i) {

    Material mat;
    if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) >
        0) {

      aiString tex;
      aiTextureMapMode mode = aiTextureMapMode_Wrap;
      sponzascene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &tex,
                                             nullptr, nullptr, nullptr, nullptr,
                                             &mode);
      string fullpath = Path + string(tex.C_Str());
      imageList[texturecount].load(fullpath.c_str());
      textureList[texturecount].CreateTexture(
          render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM,
          imageList[texturecount].mWidth, imageList[texturecount].mHeight, 1,
          8);
      mat.mChoose[MATERIALMAP_INDEX_COLOR] = 0.0;
      mat.mTextureIndex[MATERIALMAP_INDEX_COLOR] =
          texturecount + 1; // always +1 since 0 is default
      diffuseindex.push_back(texturecount);
      ++texturecount;
    } else {
      sponzascene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
      mat.mAlbedo.x = color[0];
      mat.mAlbedo.y = color[1];
      mat.mAlbedo.z = color[2];
      //		cout << color[0] << "," << color[1] << "," << color[2]
      //<< endl;
    }
    if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_NORMALS) >
        0) {

      aiString tex;
      sponzascene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &tex);
      //		cout << "Diffuse: " << tex.C_Str() << endl;
      string fullpath = Path + string(tex.C_Str());
      //		cout << fullpath << endl;
      imageList[texturecount].load(fullpath.c_str());
      textureList[texturecount].CreateTexture(
          render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM,
          imageList[texturecount].mWidth, imageList[texturecount].mHeight, 1,
          8);
      //	textureList[texturecount].addSahderResorceView(srvheap);
      mat.mChoose[MATERIALMAP_INDEX_NORMAL] = 1.0;
      mat.mTextureIndex[MATERIALMAP_INDEX_NORMAL] =
          texturecount + 1; // always +1 since 0 is default
      normalindex.push_back(texturecount);
      ++texturecount;
    }

    if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_SHININESS) >
        0) {

      aiString tex;
      sponzascene->mMaterials[i]->GetTexture(aiTextureType_SHININESS, 0, &tex);
      //			cout << "Diffuse: " << tex.C_Str() << endl;
      string fullpath = Path + string(tex.C_Str());
      //			cout << fullpath << endl;
      imageList[texturecount].load(fullpath.c_str());
      textureList[texturecount].CreateTexture(
          render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM,
          imageList[texturecount].mWidth, imageList[texturecount].mHeight, 1,
          8);
      //	textureList[texturecount].addSahderResorceView(srvheap);
      mat.mChoose[MATERIALMAP_INDEX_ROUGHNESS] = 0.0;
      mat.mTextureIndex[MATERIALMAP_INDEX_ROUGHNESS] =
          texturecount + 1; // always +1 since 0 is default
      linearindex.push_back(texturecount);

      ++texturecount;
    }

    if (sponzascene->mMaterials[i]->GetTextureCount(aiTextureType_SPECULAR) >
        0) {

      aiString tex;
      sponzascene->mMaterials[i]->GetTexture(aiTextureType_SPECULAR, 0, &tex);
      //		cout << "Diffuse: " << tex.C_Str() << endl;
      string fullpath = Path + string(tex.C_Str());
      //			cout << fullpath << endl;
      imageList[texturecount].load(fullpath.c_str());
      textureList[texturecount].CreateTexture(
          render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM,
          imageList[texturecount].mWidth, imageList[texturecount].mHeight, 1,
          8);
      //	textureList[texturecount].addSahderResorceView(srvheap);
      mat.mChoose[MATERIALMAP_INDEX_METALIC] = 0.0;
      mat.mTextureIndex[MATERIALMAP_INDEX_METALIC] =
          texturecount + 1; // always +1 since 0 is default
      linearindex.push_back(texturecount);
      ++texturecount;
    }

    materialList[i] = mat;
  }
  // sponzascene->
  Transform trans;
  trans.setScale(0.1, 0.1, 0.1);
  trans.setAngle(0, 90, 0);
  trans.CacNewTransform();
  matricesList[0] = trans.getMatrices();
  for (int i = 0; i < sponzascene->mNumMeshes; ++i) {
    meshList[i].loadMesh(sponzascene->mMeshes[i], render, cmdalloc[0],
                         cmdlist[0]);
    indirectMeshList[i] = meshList[i].getIndirectData();
    if (sponzascene->mMeshes[i]->mMaterialIndex == 0)
      cout << "Default Material" << endl;
    objectList[i].mMeshID = i;
    objectList[i].mMaterialID = sponzascene->mMeshes[i]->mMaterialIndex;
    objectList[i].mMatricesID = 0;
  }
  // meshList[0].loadMesh(sponzascene->mMeshes[0], render, cmdalloc, cmdlist);

  gaussdata = curve.generateNormalizeWeight();

  gaussianconst.createConstantBuffer(render.mDevice, srvheap,
                                     sizeof(GaussionWeight));
  gaussianconst.maptoCpu();
  gaussianconst.updateBufferfromCpu(&gaussdata, sizeof(GaussionWeight));
  gaussianconst.unMaptoCpu();

  cmdalloc[0].reset();
  cmdlist[0].reset(Pipeline());
  cmdlist[0].resourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[0].resourceTransition(normalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[0].resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

  cmdlist[0].resourceTransition(indirecrtMeshBuffer,
                                D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[0].resourceTransition(materialBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[0].resourceTransition(matricesBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[0].resourceTransition(objectBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[0].resourceTransition(scenedataBuffer,
                                D3D12_RESOURCE_STATE_COPY_DEST);

  cmdlist[0].resourceTransition(pointLightBuffer,
                                D3D12_RESOURCE_STATE_COPY_DEST, true);

  cmdlist[0].updateBufferData(vertexBuffer, mesh->mVertices,
                              mesh->mNumVertices * 3 * sizeof(float));
  cmdlist[0].updateBufferData(indexBuffer, indexdata.data(),
                              mesh->mNumFaces * 3 * sizeof(unsigned int));
  cmdlist[0].updateBufferData(normalBuffer, mesh->mNormals,
                              mesh->mNumVertices * 3 * sizeof(float));

  cmdlist[0].updateBufferData(indirecrtMeshBuffer, indirectMeshList.data(),
                              indirectMeshList.size() *
                                  sizeof(IndirectMeshData));
  cmdlist[0].updateBufferData(materialBuffer, materialList.data(),
                              materialList.size() * sizeof(Material));
  cmdlist[0].updateBufferData(matricesBuffer, matricesList.data(),
                              matricesList.size() * sizeof(Matrices));
  cmdlist[0].updateBufferData(objectBuffer, objectList.data(),
                              objectList.size() * sizeof(Object));
  cmdlist[0].updateBufferData(pointLightBuffer, pointLightList.data(),
                              pointLightList.size() * sizeof(PointLightData));

  cmdlist[0].updateBufferData(scenedataBuffer, &sceneData, sizeof(SceneData));

  cmdlist[0].resourceTransition(scenedataBuffer,
                                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

  cmdlist[0].resourceTransition(indirecrtMeshBuffer,
                                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
  cmdlist[0].resourceTransition(materialBuffer,
                                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
  cmdlist[0].resourceTransition(objectBuffer,
                                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
  cmdlist[0].resourceTransition(matricesBuffer,
                                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  cmdlist[0].resourceTransition(
      pointLightBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

  cmdlist[0].resourceTransition(
      vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  cmdlist[0].resourceTransition(
      normalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  cmdlist[0].resourceTransition(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
  cmdlist[0].resourceTransition(indirectGeoCmdBuffer,
                                D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

  // start to load image data to texture
  cmdlist[0].resourceTransition(DefualtTexture, D3D12_RESOURCE_STATE_COPY_DEST,
                                true);
  cmdlist[0].updateTextureData(DefualtTexture, &defaultnormal);
  cmdlist[0].resourceTransition(
      DefualtTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

  for (int i = 0; i < texturecount; ++i)
    cmdlist[0].resourceTransition(textureList[i],
                                  D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[0].setBarrier();
  for (int i = 0; i < texturecount; ++i)
    cmdlist[0].updateTextureData(textureList[i], imageList[i].mData, 0, 1);

  for (int i = 0; i < texturecount; ++i)
    cmdlist[0].resourceTransition(textureList[i],
                                  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  cmdlist[0].setBarrier();

  cmdlist[0].resourceTransition(HDRBuffer,
                                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                true);

  cmdlist[0].close();
  render.executeCommands(&cmdlist[0]);
  Fence tempfence;
  tempfence.initialize(render);
  render.insertSignalFence(tempfence);
  render.waitFenceIncreament(tempfence);
  tempfence.release();

  for (int i = 0; i < diffuseindex.size(); ++i) {
    render.generateMipMapOffline(
        textureList[diffuseindex[i]],
        MIP_MAP_GEN_SRGB_ALPHA_MASK_LINEAR_GAUSSIAN_CLAMP, 1);
  }

  for (int i = 0; i < linearindex.size(); ++i) {
    render.generateMipMapOffline(textureList[linearindex[i]],
                                 MIP_MAP_GEN_RGBA_LINEAR_GAUSSIAN_CLAMP, 1);
  }
  for (int i = 0; i < normalindex.size(); ++i) {
    render.generateMipMapOffline(textureList[normalindex[i]],
                                 MIP_MAP_GEN_RGBA_NORMAL_GAUSSIAN_CLAMP, 1);
  }

  pre = std::chrono::high_resolution_clock::now();
}

void releaseRender() {

  render.waitFenceIncreament(fences[0]);
  render.waitFenceIncreament(fences[1]);
  render.waitFenceIncreament(fences[2]);

  ImGui_ImplDX12_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  imgui_srvheap.release();

  pointLightIndicesBuffer.release();
  blockLightCountBuffer.release();
  scenedataBuffer.release();
  lightCullingPipe.release();
  lightCullingSig.realease();
  fowardPlusPipe.release();
  fowardPlusSig.realease();
  fowardCmdSig.release();
  DepthTexture.release();

  uploadbuffer.release();

  combinePipe.release();
  combinesig.realease();

  gaussiansig.realease();
  brightextsig.realease();
  gaussianPipeline.release();
  brightExtractPipeline.release();
  // FinalBuffer.release();
  gaussianconst.release();
  TempBuffer.release();
  BloomBuffer.release();
  HDRBuffer.release();
  pointLightBuffer.release();
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
  for (int i = 0; i < swapChainCount; ++i) {
    fences[i].release();
    cmdlist[i].release();
    cmdalloc[i].release();
  }
  render.releaseSwapChain();
  render.release();
}

void update() {
  std::chrono::high_resolution_clock::time_point t =
      std::chrono::high_resolution_clock::now();

  std::chrono::duration<float> delta = t - pre;
  pre = t;

  float dir = -1;
  if (moveLight) {
    std::vector<std::thread> workers(4);
    unsigned int offset = pointLightNum / 4;
    for (unsigned int id = 0; id < 4; ++id) {
      workers[id] = std::move(std::thread([&,id]() {
        for (int i = 0; i < offset; ++i) {
          pointLights[i + (UINT)id * offset].addPosition(
              0, 0, vel[i + (UINT)id * offset] * delta.count());
          if (abs(pointLights[i + (UINT)id * offset]
                      .getLightData()
                      ->mPosition.z) >
              120.0f)
            vel[i + (UINT)id * offset] *= -1;
          pointLightList[i + (UINT)id * offset] =
              *pointLights[i + (UINT)id * offset].getLightData();
        }
      }));
    }
    /*for (int i = 0; i < pointLightNum; ++i) {

      pointLights[i].addPosition(0, 0, vel[i] * delta.count());
      if (abs(pointLights[i].getLightData()->mPosition.z) > 120.0f)
        vel[i] *= -1;
      pointLightList[i] = *pointLights[i].getLightData();
    }*/
    for (unsigned int id = 0; id < 4; ++id) {
      if (workers[id].joinable()) {
        workers[id].join();
      }
    }
  }
  //	cout << delta.count() << endl;

  gamecamera.updateViewProj();
  //	cameraBuffer.updateBufferfromCpu(gamecamera.getMatrix(),
  // sizeof(ViewProjection));
  frameIndex = render.getCurrentSwapChainIndex();

  ImGui_ImplDX12_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  {

    ImGui::Begin("Multiple Point light shadow map: ");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::SliderInt("Visible Light Number", &visibleLightNum, 4,
                     pointLightNum);
    ImGui::Checkbox("Move Light", &moveLight);
    ImGui::Checkbox("Debug Block Light Count", &sceneData.debugDraw);
    ImGui::End();
  }
  sceneData.pointLightNum = visibleLightNum;
  render.waitFence(fences[frameIndex]);
  if (fences[frameIndex].finished())
    uploadbuffer.freeAllocateUntilFrame(fences[frameIndex].getValue());
  fences[frameIndex].updateValue(framenumber);
  uploadbuffer.setCurrentFrameNumber(framenumber);

  cmdalloc[frameIndex].reset();
  cmdlist[frameIndex].reset(GeoCmdGenPipeline);
  cmdlist[frameIndex].bindDescriptorHeaps(&srvheap, &samplerheap);

  cmdlist[frameIndex].resourceTransition(pointLightBuffer,
                                         D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[frameIndex].resourceTransition(scenedataBuffer,
                                         D3D12_RESOURCE_STATE_COPY_DEST);
  cmdlist[frameIndex].resourceTransition(
      indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_COPY_DEST); // reset count

  cmdlist[frameIndex].resourceTransition(
      pointLightIndicesBuffer, D3D12_RESOURCE_STATE_COPY_DEST); // reset count

  cmdlist[frameIndex].resourceTransition(
      cameraBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true); // reset count
  cmdlist[frameIndex].setCounterforStructeredBuffer(indirectGeoCmdBuffer, 0);

  cmdlist[frameIndex].updateBufferData(
      uploadbuffer, pointLightBuffer, pointLightList.data(),
      pointLightList.size() * sizeof(PointLightData));
  UINT count = 0;
  cmdlist[frameIndex].updateBufferData(uploadbuffer, pointLightIndicesBuffer,
                                       &count, sizeof(UINT));

  cmdlist[frameIndex].updateBufferData(uploadbuffer, cameraBuffer,
                                       gamecamera.getMatrix(),
                                       sizeof(ViewProjection));
  cmdlist[frameIndex].updateBufferData(scenedataBuffer, &sceneData,
                                       sizeof(SceneData));

  cmdlist[frameIndex].resourceTransition(
      scenedataBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                           D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  cmdlist[frameIndex].resourceTransition(indirectGeoCmdBuffer,
                                         D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  cmdlist[frameIndex].resourceTransition(
      cameraBuffer,
      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER); // reset count
  cmdlist[frameIndex].resourceTransition(
      pointLightBuffer,
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
      true);

  cmdlist[frameIndex].bindComputeRootSigature(geoCmdGenRootSig);
  cmdlist[frameIndex].dispatch(objectList.size() / objectCullingSize + 1, 1, 1);
  cmdlist[frameIndex].resourceTransition(
      indirectGeoCmdBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

  cmdlist[frameIndex].setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  cmdlist[frameIndex].resourceTransition(
      DepthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

  // pre pass
  cmdlist[frameIndex].bindGraphicsRootSigature(rootsig);
  cmdlist[frameIndex].setViewPort(viewport);
  cmdlist[frameIndex].setScissor(scissor);

  cmdlist[frameIndex].bindDepthStencilBufferOnly(DepthTexture);
  cmdlist[frameIndex].clearDepthStencil(DepthTexture);

  cmdlist[frameIndex].bindPipeline(GeoDrawPipeline);
  cmdlist[frameIndex].bindGraphicsRootSigature(geoDrawSig);
  cmdlist[frameIndex].executeIndirect(
      geomCmdSig, objectList.size(), indirectGeoCmdBuffer, 0,
      indirectGeoCmdBuffer, indirectGeoCmdBuffer.mBufferSize - sizeof(UINT));
  cmdlist[frameIndex].resourceTransition(
      DepthTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

  // Light culling pass
  cmdlist[frameIndex].resourceTransition(pointLightIndicesBuffer,
                                         D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  cmdlist[frameIndex].resourceTransition(
      blockLightCountBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
  cmdlist[frameIndex].bindComputeRootSigature(lightCullingSig);
  cmdlist[frameIndex].bindPipeline(lightCullingPipe);
  cmdlist[frameIndex].dispatch(blockWidthCount, blockHeightCount, 1);

  cmdlist[frameIndex].resourceTransition(
      pointLightIndicesBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  cmdlist[frameIndex].resourceTransition(
      blockLightCountBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  // floward pass
  cmdlist[frameIndex].resourceTransition(DepthTexture,
                                         D3D12_RESOURCE_STATE_DEPTH_READ);

  cmdlist[frameIndex].resourceTransition(
      HDRBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

  cmdlist[frameIndex].bindGraphicsRootSigature(rootsig);
  cmdlist[frameIndex].setViewPort(viewport);
  cmdlist[frameIndex].setScissor(scissor);

  cmdlist[frameIndex].bindRenderTargetsDepthStencil(HDRBuffer, DepthTexture);
  cmdlist[frameIndex].clearRenderTarget(HDRBuffer);
  cmdlist[frameIndex].bindPipeline(fowardPlusPipe);
  cmdlist[frameIndex].bindGraphicsRootSigature(fowardPlusSig);
  cmdlist[frameIndex].executeIndirect(
      fowardCmdSig, objectList.size(), indirectGeoCmdBuffer, 0,
      indirectGeoCmdBuffer, indirectGeoCmdBuffer.mBufferSize - sizeof(UINT));
  cmdlist[frameIndex].resourceTransition(
      HDRBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                     D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  // Post processing pass

  cmdlist[frameIndex].resourceTransition(BloomBuffer,
                                         D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  cmdlist[frameIndex].resourceTransition(
      TempBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
  cmdlist[frameIndex].bindComputeRootSigature(brightextsig);
  cmdlist[frameIndex].bindPipeline(brightExtractPipeline);
  cmdlist[frameIndex].bindComputeConstant(0, &bloomthreash);
  cmdlist[frameIndex].dispatch((BloomBuffer.textureDesc.Width / 256) + 1,
                               BloomBuffer.textureDesc.Height, 1);

  cmdlist[frameIndex].resourceTransition(
      BloomBuffer,
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
      true);
  // gaussian blur use temp to store hor res

  // TempBuffer
  unsigned int direc = 0;
  unsigned int direc2 = 1;
  cmdlist[frameIndex].bindComputeRootSigature(gaussiansig);
  cmdlist[frameIndex].bindPipeline(gaussianPipeline);
  cmdlist[frameIndex].bindComputeConstant(1, &direc);
  cmdlist[frameIndex].bindComputeResource(2, BloomBuffer);
  cmdlist[frameIndex].bindComputeResource(3, TempBuffer);
  cmdlist[frameIndex].dispatch((BloomBuffer.textureDesc.Width / 256) + 1,
                               BloomBuffer.textureDesc.Height, 1);

  //	cmdlist.dispatch(BloomBuffer.textureDesc.Width,
  //(BloomBuffer.textureDesc.Height / 256) + 1, 1);

  cmdlist[frameIndex].resourceTransition(
      TempBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
  cmdlist[frameIndex].resourceTransition(
      BloomBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
  //	direc = 1;
  cmdlist[frameIndex].bindComputeRootSigature(gaussiansig);
  cmdlist[frameIndex].bindPipeline(gaussianPipeline);
  cmdlist[frameIndex].bindComputeConstant(1, &direc2);
  cmdlist[frameIndex].bindComputeResource(2, TempBuffer);
  cmdlist[frameIndex].bindComputeResource(3, BloomBuffer);
  cmdlist[frameIndex].dispatch((BloomBuffer.textureDesc.Height / 256) + 1,
                               BloomBuffer.textureDesc.Width, 1);

  cmdlist[frameIndex].resourceTransition(
      BloomBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                       D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

  cmdlist[frameIndex].swapChainBufferTransition(
      render.mSwapChainRenderTarget[frameIndex],
      D3D12_RESOURCE_STATE_RENDER_TARGET, true);
  cmdlist[frameIndex].bindRenderTarget(
      render.mSwapChainRenderTarget[frameIndex]);
  cmdlist[frameIndex].setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  cmdlist[frameIndex].bindPipeline(combinePipe);
  cmdlist[frameIndex].bindGraphicsRootSigature(combinesig);
  cmdlist[frameIndex].drawInstance(3, 1, 0, 0);

  cmdlist[frameIndex].bindDescriptorHeaps(&imgui_srvheap);
  ImGui::Render();
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
                                cmdlist[frameIndex].mDx12CommandList);
  cmdlist[frameIndex].swapChainBufferTransition(
      render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT,
      true);

  cmdlist[frameIndex].close();
  render.executeCommands(&cmdlist[frameIndex]);
  render.present();

  render.insertSignalFence(fences[frameIndex]);
  ++framenumber;
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
  curxoffet = xpos - curxpos;
  curyoffet = ypos - curypos;

  curxpos = xpos;
  curypos = ypos;

  if (press)
    gamecamera.addAngle(curxoffet / 5.0, curyoffet / 5.0);
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    press = true;
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    press = false;
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  if (yoffset == 1)
    camera.addZoom(1);
  if (yoffset == -1)
    camera.addZoom(-1);
}
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {

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
int main() {
  int testnumber = 3;
  windows.initialize(1600, 900, "FowardPlus");
  windows.openWindow();
  glfwSetCursorPosCallback(windows.mWindow, cursor_pos_callback);
  glfwSetMouseButtonCallback(windows.mWindow, mouse_button_callback);
  glfwSetScrollCallback(windows.mWindow, scroll_callback);
  glfwSetKeyCallback(windows.mWindow, key_callback);

  int limit = 10000;
  int count = 0;
  initializeRender();
  loadAsset();
  while (windows.isRunning()) {

    windows.pollInput();
    update();
  }
  releaseRender();
  windows.closeWindow();
  windows.termianate();
  return 0;
}