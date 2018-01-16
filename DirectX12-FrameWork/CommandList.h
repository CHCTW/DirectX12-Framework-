#pragma once
#include "d3dx12.h"
class Pipeline;
class CommandAllocator;
class RenderTarget;
class Buffer;
class Texture;
class RootSignature;
class ViewPort;
class Scissor;
class DescriptorHeap;
class Resource;
class CubeRenderTarget;
class CommandSignature;
class QueryHeap;


class CommandList
{
public:
	CommandList();
	bool initial(ID3D12Device* device,CommandAllocator &alloc);
	void release();
	bool reset(Pipeline& pipeline);
	bool reset();
	bool close();
	void bindGraphicsRootSigature(RootSignature& rootsig,bool bindresource = true);
	void bindComputeRootSigature(RootSignature& rootsig, bool bindresource = true);
	void bindGraphicsResource(UINT rootindex, Resource& res,UINT uavmiplevel);
	void bindComputeResource(UINT rootindex, Resource& res, UINT uavmiplevel);
	void bindGraphicsResource(UINT rootindex, Resource& res);
	void bindComputeResource(UINT rootindex, Resource& res);
	void bindGraphicsConstant(UINT rootindex, void const * ConstData);
	void bindComputeConstant(UINT rootindex, void const * ConstData);


	void setViewPort(ViewPort& viewport);
	void setScissor(Scissor & scissor);
	void setTopolgy(D3D_PRIMITIVE_TOPOLOGY topo);
	void bindDescriptorHeaps(DescriptorHeap* desheap1, DescriptorHeap* desheap2 = nullptr);
	void bindVertexBuffer(Buffer& buffer);
	void bindVertexBuffers(UINT num, Buffer const * buffer);
	template <typename... T>
	void bindVertexBuffers(T&&... buffers) {
		vertexbindhelp({ std::forward<T>(buffers)... });
	}
	void bindIndexBuffer(Buffer& buffer);
	void bindRenderTarget(RenderTarget & rt,UINT miplevel = 0);
	void bindCubeRenderTarget(CubeRenderTarget & crt, UINT face,UINT level = 0);

	void bindDepthStencilBufferOnly(Texture& dsbuffer, UINT miplevel = 0);


	void bindPipeline(Pipeline& pipeline);
	void drawInstance(UINT vertexcount, UINT instancecount, UINT vertexstart, UINT instancestart);
	void drawIndexedInstanced(UINT indexcount, UINT instancecount, UINT indexstart, UINT instancestart, UINT vertexstart = 0);
	void dispatch(UINT groupCountX, UINT groupCountY, UINT groupCountZ);
	void executeIndirect(CommandSignature &commandsig, UINT commandcount, Buffer &commandbuffer, unsigned long long offset);
	void executeIndirect(CommandSignature &commandsig, UINT commandcount, Buffer &commandbuffer, UINT offset, Buffer &countbuffer, UINT countoffset);
	void beginQuery(QueryHeap& queryheap, D3D12_QUERY_TYPE type, UINT index);
	void endQuery(QueryHeap& queryheap, D3D12_QUERY_TYPE type, UINT index);
	void resolveQuery(QueryHeap& queryheap, D3D12_QUERY_TYPE type, UINT start, UINT numquery, Buffer resbuf, UINT64 offset);
	void setPrediction(Buffer& res, UINT64 offset, D3D12_PREDICATION_OP type);
	void unsetPrediction(D3D12_PREDICATION_OP type);


	void clearRenderTarget(RenderTarget &rt, const float *color,UINT miplevel = 0);
	void clearRenderTarget(RenderTarget &rt, UINT miplevel = 0);
	void clearDepthStencil(RenderTarget &rt, D3D12_CLEAR_FLAGS flag = D3D12_CLEAR_FLAG_DEPTH, float depth = 1.0f, UINT stencil = 1,UINT miplevel = 0);
	void clearDepthStencil(Texture &dsbuffer, D3D12_CLEAR_FLAGS flag = D3D12_CLEAR_FLAG_DEPTH, float depth = 1.0f, UINT stencil = 1, UINT miplevel = 0);

	void clearcubeRenderTarget(CubeRenderTarget &crt, UINT face, UINT level = 0);
	void clearcubeDepthStencil(CubeRenderTarget &crt, UINT face, UINT level = 0,D3D12_CLEAR_FLAGS flag = D3D12_CLEAR_FLAG_DEPTH, float depth = 1.0f, UINT stencil = 1);
	
	void resourceBarrier(Resource& res, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
	void resourceTransition(Resource& res,D3D12_RESOURCE_STATES stataf, bool barrier = false,UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
	void setBarrier();

	// This one should not be used after finish frame buffering, or could change this to use via swapchain's render target
    void resourceBarrier(ID3D12Resource* res, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
	void renderTargetBarrier(RenderTarget &rt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
	void depthBufferBarrier(RenderTarget &rt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
	void cubeRenderTargetBarrier(CubeRenderTarget &crt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
	void cubeDepthBufferBarrier(CubeRenderTarget &crt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);



	bool updateTextureData(Texture& texture, void  const * data);

	bool updateTextureCubeData(Texture& texture, void  const ** data);
	bool updateBufferData(Buffer& buffer, void  const  * data, UINT datasize);
	bool setCounterforStructeredBuffer(Buffer& buffer, UINT value);
	D3D12_COMMAND_LIST_TYPE mType;
	ID3D12GraphicsCommandList* mDx12CommandList;
	ID3D12CommandAllocator* mDx12Allocater;


	RootSignature* mCurrentBindGraphicsRootSig;
	RootSignature* mCurrentBindComputeRootSig;
private:
	vector<D3D12_RESOURCE_BARRIER> mAccuBarriers;
	template <typename T>
	void vertexbindhelp(std::initializer_list<T>&& bufferlist) {
		// do something
		//mDx12CommandList->IASetVertexBuffers(0,a.size())
	//	cout << bufferlist.size() << endl;

		UINT i = 0;
		D3D12_VERTEX_BUFFER_VIEW  vbs[D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		//for (UINT i = 0; i < bufferlist.size(); ++i)
		//	vbs[i] = bufferlist[i].mVertexBuffer;
	//	mDx12CommandList->IASetVertexBuffers(0, bufferlist.size(), vbs);
		for (auto buffer : bufferlist)
		{
			vbs[i] = buffer.mVertexBuffer;
			++i;
		}
		mDx12CommandList->IASetVertexBuffers(0, (UINT)bufferlist.size(), vbs);

	}
};