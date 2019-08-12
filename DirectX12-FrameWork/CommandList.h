#pragma once
#include "d3dx12.h"
#include <vector>
class Pipeline;
class CommandAllocator;
class Buffer;
class Texture;
class RootSignature;
class ViewPort;
class Scissor;
class DescriptorHeap;
class GPUResource;
class CommandSignature;
class QueryHeap;
class SwapChainBuffer;
class DynamicUploadBuffer;
class VolatileConstantBuffer;
class Sampler;
class CommandList
{
public:
	CommandList();
	bool initial(ID3D12Device* device,CommandAllocator &alloc);
	void release();
	bool reset(Pipeline& pipeline);
	bool reset();
	bool resetwithAllocate(Pipeline& pipeline);
	bool resetwithAllocate();

	bool close();
	void bindGraphicsRootSigature(RootSignature& rootsig,bool bindresource = true);
	void bindComputeRootSigature(RootSignature& rootsig, bool bindresource = true);
	void bindGraphicsResource(UINT rootindex, GPUResource& res,UINT uavmiplevel);
	void bindComputeResource(UINT rootindex, GPUResource& res, UINT uavmiplevel);
	void bindGraphicsResource(UINT rootindex, GPUResource& res);
	void bindComputeResource(UINT rootindex, GPUResource& res);
	void bindGraphicsSampler(UINT rootindex, Sampler& sampler);
	void bindComputeSampler(UINT rootindex, Sampler& sampler);

	
	
	void bindGraphicsResource(UINT rootindex, const VolatileConstantBuffer res);
	void bindComputeResource(UINT rootindex, const VolatileConstantBuffer res);
	


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
	//void bindRenderTarget(RenderTarget & rt,UINT miplevel = 0);
	/*************************/
	void bindRenderTarget(SwapChainBuffer & rt);
	
	void bindRenderTarget(SwapChainBuffer & rt,Texture & depthstencilbuffer,UINT depthstecmip = 0);
	void clearRenderTarget(SwapChainBuffer &rt, const float *color);

	void bindDepthStencilBufferOnly(Texture& dsbuffer, UINT miplevel = 0);
	// should only use texture here, try to figure out not template version
	/*template <typename... T,typename U, typename = void>
	void bindRenderTargetsOnly(Texture&... rendertargets, U mip = 0)
	{
		bindrendertargetsonlyhelp({ std::forward<Texture>(rendertargets)... }, 0);
	}*/

	void bindRenderTargetsOnly(Texture& t1, UINT mip = 0);
	void bindRenderTargetsOnly(Texture& t1, Texture& t2, UINT mip = 0);
	void bindRenderTargetsOnly(Texture& t1, Texture& t2, Texture& t3, UINT mip = 0);
	void bindRenderTargetsOnly(Texture& t1, Texture& t2, Texture& t3, Texture& t4, UINT mip = 0);
	void bindRenderTargetsDepthStencil(Texture& t1, Texture& ds, UINT retmip = 0, UINT dsmip = 0);
	void bindRenderTargetsDepthStencil(Texture& t1, Texture& t2, Texture& ds, UINT retmip = 0, UINT dsmip = 0);
	void bindRenderTargetsDepthStencil(Texture& t1, Texture& t2, Texture& t3, Texture& ds, UINT retmip = 0, UINT dsmip = 0);
	void bindRenderTargetsDepthStencil(Texture& t1, Texture& t2, Texture& t3, Texture& t4, Texture& ds, UINT retmip = 0, UINT dsmip = 0);

	/********************************/

	//void bindCubeRenderTarget(CubeRenderTarget & crt, UINT face,UINT level = 0);



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


	//void clearRenderTarget(RenderTarget &rt, const float *color,UINT miplevel = 0);
	//void clearRenderTarget(RenderTarget &rt, UINT miplevel = 0);
	//void clearDepthStencil(RenderTarget &rt, D3D12_CLEAR_FLAGS flag = D3D12_CLEAR_FLAG_DEPTH, float depth = 1.0f, UINT stencil = 1,UINT miplevel = 0);

	/******************************************/
	void clearDepthStencil(Texture &dsbuffer, D3D12_CLEAR_FLAGS flag = D3D12_CLEAR_FLAG_DEPTH, float depth = 1.0f, UINT stencil = 1, UINT miplevel = 0);
	void clearRenderTarget(Texture &rt, const float *color, UINT miplevel = 0);
	void clearRenderTarget(Texture &rt, UINT miplevel = 0);




	/***********************************/
	//void clearcubeRenderTarget(CubeRenderTarget &crt, UINT face, UINT level = 0);
	//void clearcubeDepthStencil(CubeRenderTarget &crt, UINT face, UINT level = 0,D3D12_CLEAR_FLAGS flag = D3D12_CLEAR_FLAG_DEPTH, float depth = 1.0f, UINT stencil = 1);
	//
	//void resourceBarrier(Resource& res, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	//	D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
	void UAVWait(GPUResource& res,bool barrier);
	void resourceTransition(GPUResource& res,D3D12_RESOURCE_STATES stataf, bool barrier = false,UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

	void swapChainBufferTransition(SwapChainBuffer& res, D3D12_RESOURCE_STATES stataf, bool barrier = false,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);


	void setBarrier();


	// this metod can only use when these 2 resource are exactly the same, or their format is comptable
	bool copyResource(GPUResource&src, GPUResource&desc);

	bool updateTextureData(DynamicUploadBuffer& upload,Texture& texture, void  const * data, UINT startlevel = 0, UINT levelnum = -1, UINT startslice = 0, UINT slicenum = -1);
	bool updateTextureData(Texture& texture, void  const * data,UINT startlevel = 0,UINT levelnum = -1 , UINT startslice = 0, UINT slicenum = -1);
	// same format and similar size copy style, probally will have different style copy
	bool copyTextureData(Texture& dsttexture, Texture& srctexture, UINT startlevel = 0, UINT levelnum = -1, UINT startslice = 0, UINT slicenum = -1);
	bool updateBufferData(Buffer& buffer, void  const  * data, UINT datasize);
	bool updateBufferData(DynamicUploadBuffer& upload,Buffer& buffer, void  const  * data, UINT64 datasize,UINT64 bufferoffset = 0);

	bool setCounterforStructeredBuffer(Buffer& buffer, UINT value);
	CommandType mType;
	ID3D12GraphicsCommandList* mDx12CommandList;
	ID3D12CommandAllocator* mDx12Allocater;


	RootSignature* mCurrentBindGraphicsRootSig;
	RootSignature* mCurrentBindComputeRootSig;
private:
	std::vector<D3D12_RESOURCE_BARRIER> mAccuBarriers;
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