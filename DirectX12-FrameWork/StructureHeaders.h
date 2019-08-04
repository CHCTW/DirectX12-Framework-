#pragma once
#include "DescriptorHeap.h"
#include "CommandAllocator.h"
#include "CommandList.h"
#include "Fence.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "RootSignature.h"
#include "Shader.h"
#include "ViewPort.h"
#include "Scissor.h"
#include "Render.h"
#include "Texture.h"
#include "Sampler.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "Image.h"
#include "CommandSignature.h"
#include "QueryHeap.h"
#include "SwapChainBuffer.h"
#include "DynamicUploadBuffer.h"
#include "DynamicConstantBuffer.h"


// Assign a name to the object to aid with debugging.
#if defined(_DEBUG)
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
	pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
	WCHAR fullName[50];
	if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
	{
		pObject->SetName(fullName);
	}
}
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{
}
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
{
}
#endif

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName(x.Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed(x[n].Get(), L#x, n)