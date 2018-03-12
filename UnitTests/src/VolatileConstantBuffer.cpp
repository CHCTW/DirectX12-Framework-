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
#include <list>
#include <random>





using namespace std;
Render render;
static const uint32_t swapChainCount = 3;
CommandAllocator cmdalloc[swapChainCount];
CommandList cmdlist[swapChainCount];
Window windows;
Pipeline pipeline;
Pipeline wirepipeline;
Fence fences[3];
unsigned long long fencevalue = 1;
HANDLE fenceEvet;
UINT swapchainIndex;

ViewPort viewport;
Scissor scissor;


float curxpos;
float curypos;
float curxoffet;
float curyoffet;
bool press = false;



DynamicConstantBuffer constantbuffer;
UINT64 framenumber = 1;

struct ParticleDrawData
{
	float x;
	float y;
	float size;
	float r;
	float g;
	float b;
	float a;
};
struct Particle
{
	float lifetime;
	float orisize;
	float currentlife;
	float velocityx;
	float velocityy;
	ParticleDrawData data;
	Particle(float x, float y, float velx, float vely, float r, float g, float b, float size, float life)
	{
		data.x = x;
		data.y = y;
		velocityx = velx;
		velocityy = vely;
		data.r = r;
		data.g = g;
		data.b = b;
		data.a = 1.0f;
		data.size = size;
		orisize = size;
		lifetime = life;
		currentlife = life;
		

	}
	void update(float deltatime)
	{
		currentlife -= deltatime;
		data.x += velocityx * deltatime;
		data.y += velocityy * deltatime;
		float rate = max(0.0f, currentlife / lifetime);
		data.a = rate;
		data.size = orisize * rate;

	}
};
std::list<Particle> particlelist;
std::uniform_real_distribution<float> colorrandom(0.0f, 1.0f);
std::uniform_real_distribution<float> velocityrandom(-0.2, 0.2f);
std::uniform_real_distribution<float> lifetimerandom(2.0f, 5.0f);
std::uniform_real_distribution<float> sizerandom(0.15f, 0.3f);
std::default_random_engine eng((std::random_device())());
std::chrono::high_resolution_clock::time_point pre;
float ndcx;
float ndcy;
Pipeline pointPipe;
RootSignature pointRoot;
float acumulattime;
void initializeRender()
{
	render.initialize();
	RenderTargetFormat retformat(true);
	render.createSwapChain(windows, swapChainCount, retformat.mRenderTargetFormat[0]);
	for (int i = 0; i < swapChainCount; ++i)
	{
		cmdalloc[i].initialize(render.mDevice);
		cmdlist[i].initial(render.mDevice, cmdalloc[i]);
	}
	fences[0].initialize(render);
	fences[1].initialize(render);
	fences[2].initialize(render);

	constantbuffer.initialize(render, 2000);


	pointRoot.mParameters.resize(1);
	pointRoot.mParameters[0].mType = PARAMETERTYPE_CBV;
	pointRoot.mParameters[0].mResCounts = 1;
	pointRoot.mParameters[0].mBindSlot = 0;
	pointRoot.mParameters[0].mVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pointRoot.initialize(render.mDevice);


	ShaderSet pointShader;
	pointShader.shaders[VS].load("Shaders/PointParticle.hlsl", "VSMain", VS);
	pointShader.shaders[PS].load("Shaders/PointParticle.hlsl", "PSMain", PS);


	pointPipe.createGraphicsPipeline(render.mDevice, pointRoot, pointShader, retformat, DepthStencilState::DepthStencilState(), BlendState::BlendState(true), RasterizerState::RasterizerState(D3D12_CULL_MODE_NONE, D3D12_FILL_MODE_SOLID), VERTEX_LAYOUT_TYPE_SPLIT_ALL, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	viewport.setup(0.0f, 0.0f, (float)windows.mWidth, (float)windows.mHeight);
	scissor.setup(0, windows.mWidth, 0, windows.mHeight);

}

void loadAsset()
{
	pre = std::chrono::high_resolution_clock::now();

}

void releaseRender()
{
	render.waitFence(fences[0]);
	render.waitFence(fences[1]);
	render.waitFence(fences[2]);

	constantbuffer.release();


	pointPipe.release();
	pointRoot.realease();


	fences[2].release();
	fences[1].release();
	fences[0].release();
	cmdlist[2].release();
	cmdlist[1].release();
	cmdlist[0].release();
	cmdalloc[2].release();
	cmdalloc[1].release();
	cmdalloc[0].release();
	render.releaseSwapChain();
	render.release();
	CloseHandle(fenceEvet);
}
void update()
{
	std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> delta = t - pre;
	pre = t;




	

	ndcx = ((curxpos / (float)windows.mWidth) - 0.5f)*2.0f;
	ndcy = (1.0f-(curypos / (float)windows.mHeight) - 0.5f)*2.0f;

	acumulattime += delta.count();


	if (acumulattime >= 0.01)
	{
		int count = acumulattime / 0.01f;
		for (int i = 0; i < count; ++i)
		{
			Particle par(ndcx, ndcy, velocityrandom(eng), velocityrandom(eng), colorrandom(eng), colorrandom(eng), colorrandom(eng), sizerandom(eng), lifetimerandom(eng));
			//	cout << ndcx << " , " << ndcy << endl;
			particlelist.push_back(par);
		}
		acumulattime = 0.0f;
	}

	auto it = particlelist.begin();
	while (it != particlelist.end())
	{
		if (it->currentlife <= 0)
		{
			it = particlelist.erase(it);
		//	cout << "Erase" << endl;
		}
		else
		{

			it->update(delta.count());
		//	cout<<"Delta " << delta.count() << endl;
			it = ++it;
		}
	}


}
void onrender()
{

	swapchainIndex = render.getCurrentSwapChainIndex();

	render.waitFence(fences[swapchainIndex]);
	if (fences[swapchainIndex].finished())
		constantbuffer.freeAllocateUntilFrame(fences[swapchainIndex].fenceValue);
	fences[swapchainIndex].updateValue(framenumber);
	constantbuffer.setCurrentFrameNumber(framenumber);
	cmdlist[swapchainIndex].resetwithAllocate(pointPipe);
	cmdlist[swapchainIndex].setViewPort(viewport);
	cmdlist[swapchainIndex].setScissor(scissor);


	cmdlist[swapchainIndex].swapChainBufferTransition(render.mSwapChainRenderTarget[swapchainIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	cmdlist[swapchainIndex].bindRenderTarget(render.mSwapChainRenderTarget[swapchainIndex]);
	const float clearColor[] = { 0.0f, 0.1f, 0.3f, 1.0f };
	cmdlist[swapchainIndex].clearRenderTarget(render.mSwapChainRenderTarget[swapchainIndex], clearColor);

	cmdlist[swapchainIndex].bindGraphicsRootSigature(pointRoot);
	cmdlist[swapchainIndex].setTopolgy(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	auto it = particlelist.begin();
	while (it != particlelist.end())
	{
		
	//	cout << it->data.size<< endl;
		cmdlist[swapchainIndex].bindGraphicsResource(0,constantbuffer.allocateVolatileConstantBuffer(&(it->data), sizeof(ParticleDrawData)));
		cmdlist[swapchainIndex].drawInstance(4, 1, 0, 0);
		++it;
	}




	cmdlist[swapchainIndex].swapChainBufferTransition(render.mSwapChainRenderTarget[swapchainIndex], D3D12_RESOURCE_STATE_PRESENT, true);
	cmdlist[swapchainIndex].close();
	render.executeCommands(&cmdlist[swapchainIndex]);
	render.present();
	//render.waitCommandsDone();
	render.insertSignalFence(fences[swapchainIndex]);
	//render.waitFenceIncreament(fences[swapchainIndex]);
	++framenumber;


}


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	curxoffet = xpos - curxpos;
	curyoffet = ypos - curypos;

	curxpos = xpos;
	curypos = ypos;


}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		press = true;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		press = false;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	
}
int main()
{
	int testnumber = 3;
	windows.initialize(1600, 900, "VolatileConstantBuffer");
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