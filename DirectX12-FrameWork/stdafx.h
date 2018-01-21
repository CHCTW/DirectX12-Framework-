// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <iostream>
#include "d3dx12.h"
//#pragma comment( lib,"dxgi")
//#pragma comment( lib,"d3d12")
//#pragma comment( lib,"d3dcompiler")
using namespace std;
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}
// TODO: reference additional headers your program requires here


// Render configuration 
static const unsigned int RTVHeapSize = 30;
static const unsigned int MaxSubmitCommandList = 30;
enum ViewType
{
	CBV,
	SRV,
	UAV,
	RTV,
	DSV,
	VIEW_COUNT
};
enum BufferType
{
	NONE = 0,
	VERTEX = 1,
	INDEX = 2,
	CONSTANT = 3,
	STRUCTERED = 4,
	STRUCTERED_COUNTER = 5
};
enum ParameterType
{
	PARAMETERTYPE_UNDEFINE,
	PARAMETERTYPE_ROOTCONSTANT,
	PARAMETERTYPE_CBV,
	PARAMETERTYPE_SRV,
	PARAMETERTYPE_UAV,
	PARAMETERTYPE_SAMPLER
};
enum ShaderType
{
	VS,
	PS,
	CS,
	GS,
	DS,
	HS,
	SHADERTYPE_COUNT
};
enum IndexBufferFormat
{
	INDEX_BUFFER_FORMAT_32BIT,
	INDEX_BUFFER_FORMAT_16BIT
};
typedef enum StructeredBufferType {
	STRUCTERED_BUFFER_TYPE_READ = 1,
	STRUCTERED_BUFFER_TYPE_READ_WRITE = 2,

} StructeredBufferType;
struct Handles
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE Gpu;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Cpu;
};
enum VertexInputLayOutType
{
	VERTEX_LAYOUT_TYPE_NONE_SPLIT,
	VERTEX_LAYOUT_TYPE_SPLIT_ALL

};
typedef D3D12_INDIRECT_ARGUMENT_DESC CommandParameter;
typedef D3D12_DRAW_ARGUMENTS DrawArgument;
typedef D3D12_DRAW_INDEXED_ARGUMENTS DrawIndexedArgument;
typedef D3D12_DISPATCH_ARGUMENTS DispathArgument;
typedef D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
typedef D3D12_INDEX_BUFFER_VIEW IndexBufferView;
typedef D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;

struct ClearValue
{
	enum ClearType
	{
		CLEAR_TYPE_CUSTOME = 0,
		CLEAR_TYPE_DEFAUTL = 1
	};
	union
	{
		FLOAT                     Color[4];
		D3D12_DEPTH_STENCIL_VALUE DepthStencil;
	};
	ClearValue() :Type(CLEAR_TYPE_CUSTOME)
	{

	}
	ClearValue(ClearType t) :Type(t)
	{

	}
	void setColor(float r, float g, float b, float a)
	{
		Color[0] = r;
		Color[1] = g;
		Color[2] = b;
		Color[3] = a;
	}
	ClearType Type;
};
struct RenderTargetFormat  // assume all render targt and depth buffer will need to create SRV 
{



	RenderTargetFormat() :mDepth(false), mNumofRenderTargets(1), mDepthStencilFormat(DXGI_FORMAT_R32_TYPELESS),
		mRenderTargetflags(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), mDepthStencilflags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), mCube(false)
	{
		mRenderTargetFormat[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
		mRenderTargetClearValue[0].setColor( 0.0,0.0,0.0,1.0 );
		if (mDepth)
		{
			mDepthStencilClearValue.DepthStencil.Depth = 1.0f;
		}
	}

	RenderTargetFormat(DXGI_FORMAT formatforrendertarget) :mDepth(false), mNumofRenderTargets(1), mDepthStencilFormat(DXGI_FORMAT_R32_TYPELESS),
		mRenderTargetflags(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), mDepthStencilflags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), mCube(false)
	{
		mRenderTargetFormat[0] = formatforrendertarget;
		mRenderTargetClearValue[0].setColor(0.0, 0.0, 0.0, 1.0);
	}



	RenderTargetFormat( bool depth) :mDepth(depth), mNumofRenderTargets(1), mDepthStencilFormat(DXGI_FORMAT_R32_TYPELESS),
		mRenderTargetflags(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), mDepthStencilflags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), mCube(false)
	{
		mRenderTargetFormat[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
		mRenderTargetClearValue[0].setColor(0.0, 0.0, 0.0, 1.0);
	}


	RenderTargetFormat(UINT rendertargetnum, DXGI_FORMAT *renderTargetFormat, bool depth = false, bool cube = false, DXGI_FORMAT depthFormat = DXGI_FORMAT_R32_TYPELESS, D3D12_RESOURCE_FLAGS rendertargetflags = D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_FLAGS depthflags = D3D12_RESOURCE_FLAG_NONE)
		:mDepth(depth), mNumofRenderTargets(rendertargetnum), mDepthStencilFormat(depthFormat),
		mRenderTargetflags(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET| rendertargetflags), mDepthStencilflags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL| depthflags), mCube(false)
	{
		for (unsigned int i = 0; i < rendertargetnum; ++i)
		{
			mRenderTargetFormat[i] = renderTargetFormat[i];
			mRenderTargetClearValue[0].setColor(0.0, 0.0, 0.0, 1.0);
		}
		if (mDepth)
		{
			mDepthStencilClearValue.DepthStencil.Depth = 1.0f;
		}
	}

	bool mDepth;
	bool mCube;
	UINT mNumofRenderTargets;
	DXGI_FORMAT mRenderTargetFormat[8];
	DXGI_FORMAT mDepthStencilFormat;
	D3D12_RESOURCE_FLAGS mRenderTargetflags;
	D3D12_RESOURCE_FLAGS mDepthStencilflags;
	ClearValue mRenderTargetClearValue[8];
	ClearValue mDepthStencilClearValue;

};

const static char *Targetchars[SHADERTYPE_COUNT]
{
	"vs_5_1","ps_5_1","cs_5_0","gs_5_1","ds_5_1","hs_5_1"
};
static unsigned int HeapSizes[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = { 3000,10,1000,1000 };
static unsigned int HeapOffset[VIEW_COUNT] = {0,1000,2000,0,0};
static FLOAT DefaultBorderColor[4] = { 0.0,0.0,0.0,0.0 };
static ClearValue DefaultClearValue(ClearValue::ClearType::CLEAR_TYPE_DEFAUTL);
//DefaultClearValue.Type = ClearValue::ClearType::CLEAR_TYPE_DEFAUTL;


// Mip_map generate type

enum Mip_Map_Generate_Type
{
	MIP_MAP_GEN_SRGB_A_BOX_CLAMP = 0,
	MIP_MAP_GEN_RGBA_LINEAR_BOX_CLAMP = 1,
	MIP_MAP_GEN_SRGB_ALPHA_MASK_LINEAR_BOX_CLAMP = 2,
	MIP_MAP_GEN_SRGB_A_GAUSSIAN_CLAMP = 3,
	MIP_MAP_GEN_RGBA_LINEAR_GAUSSIAN_CLAMP = 4,
	MIP_MAP_GEN_SRGB_ALPHA_MASK_LINEAR_GAUSSIAN_CLAMP = 5,
	MIP_MAP_GEN_COUNT
};


const static char *MipMapShadersName[MIP_MAP_GEN_COUNT]
{
	"DownScale_Srgb_A_Box_Clamp.hlsl",
	"DownScale_Rgba_Linear_Box_Clamp.hlsl",
	"DownScale_Srgb_Alpha_Mask_Box_Clamp.hlsl",
	"DownScale_Srgb_A_Gaussian_Clamp.hlsl",
	"DownScale_Rgba_Linear_Gaussian_Clamp.hlsl",
	"DownScale_Srgb_Alpha_Mask_Gaussian_Clamp.hlsl",
};