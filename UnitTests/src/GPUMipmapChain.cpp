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
#include "QuadPatch.h"
#include "Mesh.h"
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
	Buffer mUVBuffer;
	Buffer mTangentBuffer;
	Buffer mIndexBuffer;
	Buffer mInstancedBuffer;

	UINT indexCount;
	UINT mNum;
};

using namespace std;
Render render;
CommandAllocator cmdalloc;
CommandList cmdlist;
Window windows;
Pipeline pipeline;
Pipeline wirepipeline;
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
vector<Texture> depthBuffer;
ShaderSet shaderset;
Assimp::Importer import;
aiScene const * scene = nullptr;
aiMesh* mesh = nullptr;

Assimp::Importer buddhaimport;


SpecCamera camera;
float curxpos;
float curypos;
float curxoffet;
float curyoffet;
bool press = false;


ObjectData Sphere;

const UINT rowcount = 3;
const UINT collomcount = 2;
UINT spherecount = rowcount*collomcount;

float radious = 10;
float heightgap = 10;
float rotationoffset = 0.0;
float radian;
float rotationspeed = 0.0005f;


bool lightmove;
ObjectData Ground;
SpotLight light;

QuadPatch patch;

Texture blockbase;
Texture blocknormal;
Texture blockmetal;
Texture blockrough;
float heightscale = 1.0f;
bool wire = false;

DescriptorHeap samplerheap;

Sampler loopsampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR,  // due to pass too many parameters, should use inline to prevent a lot of copy
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP);
void initializeRender()
{


	//Camera cam;



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

	rootsig.mParameters.resize(6);
	rootsig.mParameters[0].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[0].mTable = false;
	rootsig.mParameters[0].mResCounts = 1;
	rootsig.mParameters[0].mBindSlot = 0;
	rootsig.mParameters[0].mResource = &cameraBuffer;
	rootsig.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[1].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[1].mResCounts = 1;
	rootsig.mParameters[1].mBindSlot = 0;
	rootsig.mParameters[2].mType = PARAMETERTYPE_CBV;
	rootsig.mParameters[2].mResCounts = 1;
	rootsig.mParameters[2].mBindSlot = 1;
	rootsig.mParameters[2].mResource = &lightBuffer;
	rootsig.mParameters[2].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[3].mType = PARAMETERTYPE_SRV;
	rootsig.mParameters[3].mResCounts = 4;
	rootsig.mParameters[3].mBindSlot = 1;
	rootsig.mParameters[3].mResource = &blockbase;
	rootsig.mParameters[3].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[4].mType = PARAMETERTYPE_SAMPLER;
	rootsig.mParameters[4].mResCounts = 1;
	rootsig.mParameters[4].mBindSlot = 0;
	rootsig.mParameters[4].mSampler = &loopsampler;
	rootsig.mParameters[4].mVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootsig.mParameters[5].mType = PARAMETERTYPE_ROOTCONSTANT;
	rootsig.mParameters[5].mResCounts = 1;
	rootsig.mParameters[5].mBindSlot = 2;
	rootsig.mParameters[5].mConstantData = &heightscale;



	rootsig.initialize(render.mDevice);

	shaderset.shaders[VS].load("Shaders/GPUMipmapChain.hlsl", "VSMain", VS);
	shaderset.shaders[PS].load("Shaders/GPUMipmapChain.hlsl", "PSMain", PS);
	//shaderset.shaders[HS].load("Shaders/GPUMipmapChain.hlsl", "HSMain", HS);
	//shaderset.shaders[DS].load("Shaders/GPUMipmapChain.hlsl", "DSMain", DS);

	pipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT, D3D12_FILL_MODE_SOLID), VERTEX_LAYOUT_TYPE_SPLIT_ALL, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	wirepipeline.createGraphicsPipeline(render.mDevice, rootsig, shaderset, retformat, DepthStencilState::DepthStencilState(true), BlendState::BlendState(), RasterizerState::RasterizerState(D3D12_CULL_MODE_FRONT, D3D12_FILL_MODE_WIREFRAME), VERTEX_LAYOUT_TYPE_SPLIT_ALL, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

	camera.setRatio((float)windows.mWidth / (float)windows.mHeight);
	camera.updateViewProj();
}

void loadAsset()
{







	/****************Continuous regist to heap, usig array of texture here***************/

	Image blockbasecolor;
	blockbasecolor.load("Assets/Textures/Sponza_Curtain_Red_diffuse.tga");
	blockbase.CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, blockbasecolor.mWidth, blockbasecolor.mHeight,1,8);
	//blockbase.addSahderResorceView(srvheap);

	Image blockn;
	blockn.load("Assets/Textures/Sponza_Curtain_Red_normal.tga");
	blocknormal.CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, blockn.mWidth, blockn.mHeight,1,8);
	//blocknormal.addSahderResorceView(srvheap);


	Image blockh;
	blockh.load("Assets/Textures/Sponza_Curtain_metallic.tga");
	blockmetal.CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, blockn.mWidth, blockn.mHeight,1,8);
	//	blockmetal.addSahderResorceView(srvheap);

	Image blockr;
	blockr.load("Assets/Textures/Sponza_Curtain_roughness.tga");
	blockrough.CreateTexture(render, srvheap, DXGI_FORMAT_R8G8B8A8_UNORM, blockn.mWidth, blockn.mHeight,1,8);
	//	blockrough.addSahderResorceView(srvheap);


	/*************************************************************/

	loopsampler.createSampler(samplerheap);

	camera.mZoom = 20;

	cameraBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(ViewProjection));
	cameraBuffer.maptoCpu();

	lightBuffer.createConstantBuffer(render.mDevice, srvheap, sizeof(SpotLightData));
	lightBuffer.maptoCpu();

	light.setRadius(1500);
	light.setIntensity(15000);
	light.setColor(1, 1, 1);
	light.setConeAngle(90);
	light.addZoom(50);
	light.addAngle(0, 90);




	patch.generatePatch(500, 500, 20, 20, Triangle, ZERO, 0, 3, 3, 5,10.0,10.0);
	Ground.mVertexBufferData.createVertexBuffer(render.mDevice, patch.mPosition.size() * sizeof(float), sizeof(float) * 3);
	Ground.mNormalBuffer.createVertexBuffer(render.mDevice, patch.mNormal.size() * sizeof(float), sizeof(float) * 3);
	Ground.mUVBuffer.createVertexBuffer(render.mDevice, patch.mUV.size() * sizeof(float), sizeof(float) * 2);
	Ground.mTangentBuffer.createVertexBuffer(render.mDevice, patch.mTangent.size() * sizeof(float), sizeof(float) * 3);
	Ground.mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int)*patch.mIndex.size());
	Ground.mNum = 1;
	Ground.mBufferData.resize(Ground.mNum);
	Ground.mPosition.resize(Ground.mNum);
	Ground.indexCount = patch.mIndex.size();
	Ground.mInstancedBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Ground.mNum, STRUCTERED_BUFFER_TYPE_READ);
	Ground.mPosition[0].setScale(1, 1, 1);
	Ground.mPosition[0].setPosition(0, -5, 0);
	Ground.mPosition[0].CacNewTransform();
	Ground.mBufferData[0].mMatrices = Ground.mPosition[0].getMatrices();
	Ground.mBufferData[0].mMaterial.mAlbedo = glm::vec3(0.55, 0.55, 0.55);
	Ground.mBufferData[0].mMaterial.mRoughness = 0.8;
	Ground.mBufferData[0].mMaterial.mMetallic = 0.8;





	// load sphere data

	import.ReadFile("Assets/newsphere.obj", aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs);
	scene = import.GetScene();
	mesh = scene->mMeshes[0];


	UINT verticnum = mesh->mNumVertices;

	if (!Sphere.mVertexBufferData.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float)))
	{
		cout << "Fail for create" << endl;
	}
	Sphere.mNormalBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	Sphere.mUVBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	Sphere.mTangentBuffer.createVertexBuffer(render.mDevice, mesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	Sphere.mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * 3 * mesh->mNumFaces);



	std::vector<unsigned int> indexdata;
	indexdata.resize(mesh->mNumFaces * 3);
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		indexdata[i * 3] = mesh->mFaces[i].mIndices[0];
		indexdata[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		indexdata[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}

	// initialize spheres position data
	Sphere.mNum = spherecount;
	Sphere.indexCount = mesh->mNumFaces * 3;
	Sphere.mBufferData.resize(Sphere.mNum);
	Sphere.mPosition.resize(Sphere.mNum);

	Sphere.mInstancedBuffer.createStructeredBuffer(render.mDevice, srvheap, sizeof(InstancedInformation), Sphere.mNum, STRUCTERED_BUFFER_TYPE_READ);

	radian = 2 * 3.14159f / rowcount;
	float rough = 1.0f / (rowcount - 1);
	float metalic = 1.0f / (collomcount - 1);
	for (int i = 0; i < Sphere.mNum; ++i)
	{
		int height = i / rowcount;
		int rowpos = i % rowcount;
		Sphere.mPosition[i].setPosition(radious*cos(i*radian), heightgap*height, radious*sin(i*radian));
		Sphere.mPosition[i].setScale(2, 2, 2);
		//Sphere.mPosition[i].setAngle(-90, 0, 0);
		Sphere.mPosition[i].CacNewTransform();
		Sphere.mBufferData[i].mMatrices = Sphere.mPosition[i].getMatrices();
		Sphere.mBufferData[i].mMaterial.mAlbedo = glm::vec3(1.0, 0.0, 0.0);
		Sphere.mBufferData[i].mMaterial.mRoughness = (rowpos)*rough;
		Sphere.mBufferData[i].mMaterial.mMetallic = height*metalic;

	}




	cmdalloc.reset();
	cmdlist.reset(Pipeline());
	cmdlist.resourceTransition(depthBuffer[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[1], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(depthBuffer[2], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdlist.resourceTransition(Sphere.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Sphere.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Sphere.mUVBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Sphere.mTangentBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Sphere.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Sphere.mInstancedBuffer, D3D12_RESOURCE_STATE_COPY_DEST);




	cmdlist.resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mUVBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mTangentBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(Ground.mInstancedBuffer, D3D12_RESOURCE_STATE_COPY_DEST);




	cmdlist.resourceTransition(blockbase, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(blocknormal, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdlist.resourceTransition(blockmetal, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdlist.resourceTransition(blockrough, D3D12_RESOURCE_STATE_COPY_DEST, true);

	cmdlist.updateBufferData(Sphere.mVertexBufferData, mesh->mVertices, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Sphere.mIndexBuffer, indexdata.data(), mesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.updateBufferData(Sphere.mNormalBuffer, mesh->mNormals, mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Sphere.mUVBuffer, mesh->mTextureCoords[0], mesh->mNumVertices * 3 * sizeof(float));
	cmdlist.updateBufferData(Sphere.mTangentBuffer, mesh->mTangents, mesh->mNumVertices * 3 * sizeof(float));

	cmdlist.updateBufferData(Sphere.mInstancedBuffer, Sphere.mBufferData.data(), Sphere.mNum * sizeof(InstancedInformation));



	cmdlist.updateBufferData(Ground.mVertexBufferData, patch.mPosition.data(), patch.mPosition.size() * sizeof(float));
	cmdlist.updateBufferData(Ground.mIndexBuffer, patch.mIndex.data(), patch.mIndex.size() * sizeof(unsigned int));
	cmdlist.updateBufferData(Ground.mNormalBuffer, patch.mNormal.data(), patch.mNormal.size() * sizeof(float));
	cmdlist.updateBufferData(Ground.mUVBuffer, patch.mUV.data(), patch.mUV.size() * sizeof(float));
	cmdlist.updateBufferData(Ground.mTangentBuffer, patch.mTangent.data(), patch.mTangent.size() * sizeof(float));


	cmdlist.updateBufferData(Ground.mInstancedBuffer, Ground.mBufferData.data(), Ground.mNum * sizeof(InstancedInformation));




	cmdlist.updateTextureData(blockbase, blockbasecolor.mData,0,1);
	cmdlist.updateTextureData(blocknormal, blockn.mData,0,1);
	cmdlist.updateTextureData(blockmetal, blockh.mData,0,1);
	cmdlist.updateTextureData(blockrough, blockr.mData,0,1);


	cmdlist.resourceTransition(blockbase, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceTransition(blocknormal, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceTransition(blockmetal, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdlist.resourceTransition(blockrough, D3D12_RESOURCE_STATE_GENERIC_READ);


	cmdlist.resourceTransition(Sphere.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Sphere.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Sphere.mUVBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Sphere.mTangentBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	cmdlist.resourceTransition(Sphere.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Sphere.mInstancedBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);



	cmdlist.resourceTransition(Ground.mVertexBufferData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mUVBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdlist.resourceTransition(Ground.mTangentBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);


	cmdlist.resourceTransition(Ground.mIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdlist.resourceTransition(Ground.mInstancedBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, true);


	cmdlist.close();
	render.executeCommands(&cmdlist);
	render.waitCommandsDone();

	render.generateMipMapOffline(blockbase, MIP_MAP_GEN_SRGB_ALPHA_MASK_LINEAR_GAUSSIAN_CLAMP, 1);
	render.generateMipMapOffline(blockmetal, MIP_MAP_GEN_RGBA_LINEAR_GAUSSIAN_CLAMP, 1);
	render.generateMipMapOffline(blockrough, MIP_MAP_GEN_RGBA_LINEAR_GAUSSIAN_CLAMP, 1);
	render.generateMipMapOffline(blocknormal, MIP_MAP_GEN_RGBA_NORMAL_BOX_CLAMP, 1);
}

void releaseRender()
{
	depthBuffer[2].release();
	depthBuffer[1].release();
	depthBuffer[0].release();
	samplerheap.release();
	blocknormal.release();
	blockbase.release();
	blockmetal.release();
	blockrough.release();


	Ground.mUVBuffer.release();
	Ground.mTangentBuffer.release();
	Ground.mNormalBuffer.release();
	Ground.mVertexBufferData.release();
	Ground.mInstancedBuffer.release();
	Ground.mIndexBuffer.release();

	lightBuffer.release();

	Sphere.mInstancedBuffer.release();
	import.FreeScene();
	cameraBuffer.release();

	Sphere.mTangentBuffer.release();
	Sphere.mUVBuffer.release();
	Sphere.mNormalBuffer.release();
	Sphere.mIndexBuffer.release();
	Sphere.mVertexBufferData.release();

	srvheap.release();
	wirepipeline.release();
	pipeline.release();
	rootsig.realease();

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
	lightBuffer.updateBufferfromCpu(light.getLightData(), sizeof(SpotLightData));
	//radian = 2 * 3.14159f / Sphere.mNum;
	rotationoffset += rotationspeed;
	for (int i = 0; i < Sphere.mNum; ++i)
	{
		int height = i / rowcount;
		int rowpos = i % rowcount;
		Sphere.mPosition[i].setPosition(radious*cos(rowpos*radian + rotationoffset), heightgap*height - (collomcount*heightgap) / 2, radious*sin(rowpos*radian + rotationoffset));

		//	Sphere.mPosition[i].setPosition(rowpos*1.5, height, 5);
		//Sphere.mPosition[i].setAngle(0, (rowpos*radian+ rotationoffset)/3.14159*180, 0);
		Sphere.mPosition[i].CacNewTransform();
		Sphere.mBufferData[i].mMatrices = Sphere.mPosition[i].getMatrices();
	}

}
void onrender()
{

	frameIndex = render.getCurrentSwapChainIndex();
	cmdalloc.reset();
	if (!wire)
		cmdlist.reset(pipeline);
	else
		cmdlist.reset(wirepipeline);



	//cmdlist.resourceBarrier(Sphere.mInstancedBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	//cmdlist.updateBufferData(Sphere.mInstancedBuffer, Sphere.mBufferData.data(), Sphere.mNum * sizeof(InstancedInformation));
	//cmdlist.resourceBarrier(Sphere.mInstancedBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);











	cmdlist.bindDescriptorHeaps(&srvheap, &samplerheap);
	cmdlist.bindGraphicsRootSigature(rootsig);
	cmdlist.setViewPort(viewport);
	cmdlist.setScissor(scissor);
	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	cmdlist.bindRenderTarget(render.mSwapChainRenderTarget[frameIndex], depthBuffer[frameIndex]);
	const float clearColor[] = { 0.0f, 0.1f, 0.3f, 1.0f };
	cmdlist.clearRenderTarget(render.mSwapChainRenderTarget[frameIndex], clearColor);
	cmdlist.clearDepthStencil(depthBuffer[frameIndex]);
	cmdlist.setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	cmdlist.bindGraphicsResource(1, Sphere.mInstancedBuffer);
	cmdlist.bindGraphicsResource(1, Ground.mInstancedBuffer);
	cmdlist.bindIndexBuffer(Ground.mIndexBuffer);
	cmdlist.bindVertexBuffers(Ground.mVertexBufferData, Ground.mNormalBuffer, Ground.mUVBuffer, Ground.mTangentBuffer);
	cmdlist.drawIndexedInstanced(Ground.indexCount, Ground.mNum, 0, 0);





	cmdlist.swapChainBufferTransition(render.mSwapChainRenderTarget[frameIndex], D3D12_RESOURCE_STATE_PRESENT, true);
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


	if (key == GLFW_KEY_LEFT)
		heightscale -= 0.01;
	if (key == GLFW_KEY_RIGHT)
		heightscale += 0.01;


	if (key == GLFW_KEY_1)
		wire = false;
	if (key == GLFW_KEY_2)
		wire = true;
}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "DisplacementMap");
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