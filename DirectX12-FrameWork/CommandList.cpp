#include "stdafx.h"
#include "StructureHeaders.h"
CommandList::CommandList():mDx12CommandList(nullptr), mDx12Allocater(nullptr), mCurrentBindGraphicsRootSig(nullptr), mCurrentBindComputeRootSig(nullptr)
{

}
bool CommandList::initial(ID3D12Device* device, CommandAllocator &alloc)
{
	HRESULT hr;
	hr = device->CreateCommandList(0, alloc.mType, alloc.mDx12Allocater, NULL, IID_PPV_ARGS(&mDx12CommandList));
	if (FAILED(hr))
		return false;
	mType = alloc.mType;
	mDx12Allocater = alloc.mDx12Allocater;
	mDx12CommandList->Close();
	return true;
}
void CommandList::release()
{
	SAFE_RELEASE(mDx12CommandList);
}
bool CommandList::reset(Pipeline& pipeline)
{
	HRESULT hr;
	hr = mDx12CommandList->Reset(mDx12Allocater,pipeline.mPipeline);
	if (FAILED(hr))
		return false;
	return true;
}
bool CommandList::reset()
{
	HRESULT hr;
	hr = mDx12CommandList->Reset(mDx12Allocater, nullptr);
	if (FAILED(hr))
		return false;
	return true;
}
void CommandList::bindPipeline(Pipeline& pipeline)
{
	mDx12CommandList->SetPipelineState(pipeline.mPipeline);
}
void CommandList::resourceBarrier(Resource& res, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
	D3D12_RESOURCE_BARRIER_FLAGS flags)
{
	
	mDx12CommandList->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(res.mResource,statbef,stataf,subresource,flags));
	res.mState = stataf;
}
void CommandList::resourceBarrier(ID3D12Resource* res, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
	D3D12_RESOURCE_BARRIER_FLAGS flags)
{

	mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res, statbef, stataf, subresource, flags));
}
void CommandList::renderTargetBarrier(RenderTarget &rt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
	D3D12_RESOURCE_BARRIER_FLAGS flags)
{

	
	for (int i = 0; i < rt.mRenderBuffers.size(); ++i)
	{
		mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rt.mRenderBuffers[i].mResource, statbef, stataf, subresource, flags));
	}
	//mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rt.mRenderBuffer, statbef, stataf, subresource, flags));
	//rt.mRenderBufferState = stataf;
}
void CommandList::depthBufferBarrier(RenderTarget &rt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,D3D12_RESOURCE_BARRIER_FLAGS flags)
{
	if (rt.mDepthBuffer.size() == 1)
	{
		mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rt.mDepthBuffer[0].mResource, statbef, stataf, subresource, flags));
	}
}


void CommandList::cubeRenderTargetBarrier(CubeRenderTarget &crt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
	D3D12_RESOURCE_BARRIER_FLAGS flags)
{
	if (crt.mType & (CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
	{
		mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(crt.mRenderBuffer[0].mResource, statbef, stataf, subresource, flags));
	}
}
void CommandList::cubeDepthBufferBarrier(CubeRenderTarget &crt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
	D3D12_RESOURCE_BARRIER_FLAGS flags)
{
	if (crt.mType & (CUBE_RENDERTAERGET_TYPE_DEPTH))
	{
		mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(crt.mDepthBuffer[0].mResource, statbef, stataf, subresource, flags));
	}
}


bool CommandList::close()
{
	HRESULT hr;
	hr = mDx12CommandList->Close();
	if (FAILED(hr))
		return false;
	return true;
}
void CommandList::bindRenderTarget(RenderTarget & rt)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpus[8] = {};
	for (int i = 0 ; i < rt.mRenderBuffers.size() ; ++i)
	{
		cpus[i] = rt.mRenderBuffers[i].mRTV.Cpu;
	}
	if(rt.mFormat.mDepth)
		mDx12CommandList->OMSetRenderTargets(rt.mRenderBuffers.size(), cpus, false, &(rt.mDepthBuffer[0].mDSV.Cpu));
	else
	mDx12CommandList->OMSetRenderTargets(rt.mRenderBuffers.size(), cpus, false, nullptr);
}

void CommandList::bindCubeRenderTarget(CubeRenderTarget & crt, UINT face,UINT level)
{
	if (crt.mType == (CUBE_RENDERTAERGET_TYPE_DEPTH| CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
	{
		mDx12CommandList->OMSetRenderTargets(1, &(crt.mFaceRTV[level].Cpu), false, &crt.mFaceDSV[level].Cpu);
	}
	else if (crt.mType == (CUBE_RENDERTAERGET_TYPE_DEPTH))
	{
		mDx12CommandList->OMSetRenderTargets(0, nullptr, false, &crt.mFaceDSV[level].Cpu);
	}
	else
		mDx12CommandList->OMSetRenderTargets(1, &(crt.mFaceRTV[level].Cpu), false, nullptr);
}

void CommandList::clearRenderTarget(RenderTarget &rt, const float *color)
{
//	CD3DX12_CPU_DESCRIPTOR_HANDLE cpus[8] = {};
	for (int i = 0; i < rt.mRenderBuffers.size(); ++i)
	{
	//	cpus[i] = rt.mRenderBuffers[i].mRTV.Cpu;

		mDx12CommandList->ClearRenderTargetView(rt.mRenderBuffers[i].mRTV.Cpu, color, 0, NULL);

	}
//	mDx12CommandList->ClearRenderTargetView(rt.mRTV.Cpu, color, 0, NULL);
}
void CommandList::clearRenderTarget(RenderTarget &rt)
{
	for (int i = 0; i < rt.mRenderBuffers.size(); ++i)
	{
		//	cpus[i] = rt.mRenderBuffers[i].mRTV.Cpu;

		mDx12CommandList->ClearRenderTargetView(rt.mRenderBuffers[i].mRTV.Cpu, rt.mFormat.mRenderTargetClearValue[i].Color, 0, NULL);

	}
}


void CommandList::clearDepthStencil(RenderTarget &rt, D3D12_CLEAR_FLAGS flag, float depth, UINT stencil)
{
	if (rt.mFormat.mDepth)
	{
		mDx12CommandList->ClearDepthStencilView(rt.mDepthBuffer[0].mDSV.Cpu, flag, depth, stencil, 0, nullptr);
	}
}

void CommandList::clearcubeRenderTarget(CubeRenderTarget &crt, UINT face, UINT level)
{
	if (crt.mType & (CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
	{
		mDx12CommandList->ClearRenderTargetView(crt.mFaceRTV[level].Cpu, crt.mRenderTargetClearValue.Color, 0, NULL);
	}

}
void CommandList::clearcubeDepthStencil(CubeRenderTarget &crt, UINT face, UINT level, D3D12_CLEAR_FLAGS flag, float depth, UINT stencil)
{
	if (crt.mType & (CUBE_RENDERTAERGET_TYPE_DEPTH))
	{
		mDx12CommandList->ClearDepthStencilView(crt.mFaceDSV[level].Cpu, flag, depth, stencil, 0, nullptr);
	}
}



bool CommandList::updateBufferData(Buffer& buffer, void  const  * data, UINT datasize)
{
	if (!buffer.mResource || !buffer.mUploadBuffer)
	{
		return false;
	}
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	//ThrowIfFailed(mResource->Map(0, &readRange, &CpuMapAddress));
	void* mapaddress;

	buffer.mUploadBuffer->Map(0, &readRange, &mapaddress);
	memcpy(mapaddress, data, datasize);
	mDx12CommandList->CopyBufferRegion(buffer.mResource, 0, buffer.mUploadBuffer, 0, datasize);
	buffer.mUploadBuffer->Unmap(0,nullptr);

	D3D12_SUBRESOURCE_DATA Data = {};
	Data.pData = data;
	Data.RowPitch =  datasize;
	Data.SlicePitch = Data.RowPitch;
	//UpdateSubresources<1>(mDx12CommandList, buffer.mResource, buffer.mUploadBuffer, 0, 0, 1, &Data);
	return true;
}
bool CommandList::setCounterforStructeredBuffer(Buffer& buffer, UINT value)
{
	if (buffer.mType != STRUCTERED_COUNTER)
	{
		cout << "Only Counter Sturctered Buffer can set counter Value";
	}

	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
										//ThrowIfFailed(mResource->Map(0, &readRange, &CpuMapAddress));
	void* mapaddress;
	buffer.mUploadBuffer->Map(0, &readRange, &mapaddress);
	char* temp = (char*)mapaddress;
	temp+= (buffer.mBufferSize-sizeof(UINT));
	memcpy(temp, &value, sizeof(UINT));
	mDx12CommandList->CopyBufferRegion(buffer.mResource, buffer.mBufferSize - sizeof(UINT), buffer.mUploadBuffer, buffer.mBufferSize - sizeof(UINT), sizeof(UINT));
	buffer.mUploadBuffer->Unmap(0, nullptr);
	return true;
}

void CommandList::setTopolgy(D3D_PRIMITIVE_TOPOLOGY topo)
{
	mDx12CommandList->IASetPrimitiveTopology(topo);
}
bool CommandList::updateTextureData(Texture& texture, void const * data)
{
	if (!texture.mResource || !texture.mUploadBuffer)
		return false;
	//D3D12_SUBRESOURCE_DATA Data = {};
	//Data.pData = data;
	//Data.RowPitch = texture.textureDesc.Width * texture.mByteSize;
	//Data.SlicePitch = Data.RowPitch * texture.textureDesc.Height;
	//UpdateSubresources(mDx12CommandList, texture.mResource, texture.mUploadBuffer, 0, 0, 1, &Data);




	D3D12_SUBRESOURCE_DATA *Data = new D3D12_SUBRESOURCE_DATA[texture.textureDesc.MipLevels];

	texture.textureDesc.MipLevels;

	unsigned w, h, total;
	w = texture.textureDesc.Width;
	h = texture.textureDesc.Height;
	total = 0;



	//D3D12_SUBRESOURCE_DATA *Data = new D3D12_SUBRESOURCE_DATA[ texture.textureDesc.MipLevels];

		for (int j = 0; j < texture.textureDesc.MipLevels; j++)
		{
			Data[j].pData = (const char *)data + total;
			Data[j].RowPitch = w * texture.mByteSize;
			Data[j].SlicePitch = h*Data[j].RowPitch;
			total += (w*h*texture.mByteSize);
			w = w / 2;
			h = h / 2;
		}
	UpdateSubresources(mDx12CommandList, texture.mResource, texture.mUploadBuffer, 0, 0, texture.textureDesc.MipLevels, Data);


	delete[]Data;





	return true;

}
bool CommandList::updateTextureCubeData(Texture& texture, void  const ** data)
{
	if (!texture.mResource || !texture.mUploadBuffer || !texture.mCubeMap)
		return false;

	texture.textureDesc.MipLevels;

	unsigned w, h, total;
	w = texture.textureDesc.Width;
	h = texture.textureDesc.Height;
	total = 0;


	
	D3D12_SUBRESOURCE_DATA *Data = new D3D12_SUBRESOURCE_DATA[6* texture.textureDesc.MipLevels];
	for (int i = 0; i < 6; ++i)
	{
		w = texture.textureDesc.Width;
		h = texture.textureDesc.Height;
		total = 0;
		for (int j = 0; j < texture.textureDesc.MipLevels; j++)
		{ 
			Data[i*texture.textureDesc.MipLevels+j].pData = (const char *)data[i]+ total; // mip level offset
			Data[i*texture.textureDesc.MipLevels + j].RowPitch = w * texture.mByteSize;
			Data[i*texture.textureDesc.MipLevels + j].SlicePitch = h*Data[i*texture.textureDesc.MipLevels + j].RowPitch;
			total += (w*h*texture.mByteSize);
			w = w / 2;
			h = h / 2;
		}
	}
	UpdateSubresources(mDx12CommandList, texture.mResource, texture.mUploadBuffer, 0, 0, 6 * texture.textureDesc.MipLevels, Data);

//	++i;
	delete[]Data;
	return true;
}
void CommandList::setViewPort(ViewPort& viewport)
{
	mDx12CommandList->RSSetViewports(1,&viewport.mViewPort);
}
void CommandList::setScissor(Scissor & scissor)
{
	mDx12CommandList->RSSetScissorRects(1, &scissor.mScissorRect);
}

void CommandList::bindGraphicsRootSigature(RootSignature& rootsig,bool bindresource)
{
	mDx12CommandList->SetGraphicsRootSignature(rootsig.mRootSignature);
	mCurrentBindGraphicsRootSig = &rootsig;


	if (bindresource)
	{

		for (int i = 0; i < rootsig.mParameters.size(); ++i)
		
		{
			if (rootsig.mParameters[i].mResource == nullptr)
				continue;
			switch (rootsig.mParameters[i].mType)
			{
			case PARAMETERTYPE_ROOTCONSTANT:
				mDx12CommandList->SetGraphicsRoot32BitConstants(i, rootsig.mParameters[i].mResCounts, rootsig.mParameters[i].mConstantData, 0);
				break;
			case PARAMETERTYPE_CBV:
				if (rootsig.mParameters[i].mTable)
				{
					mDx12CommandList->SetGraphicsRootDescriptorTable(i, rootsig.mParameters[i].mResource->mCBV.Gpu);
				}
				else
					mDx12CommandList->SetGraphicsRootConstantBufferView(i, rootsig.mParameters[i].mResource->GpuAddress);
				break;
			case PARAMETERTYPE_SRV:
				if (rootsig.mParameters[i].mTable)
				{
					mDx12CommandList->SetGraphicsRootDescriptorTable(i, rootsig.mParameters[i].mResource->mSRV.Gpu);
				}
				else
					mDx12CommandList->SetGraphicsRootShaderResourceView(i, rootsig.mParameters[i].mResource->GpuAddress);
				break;
			case PARAMETERTYPE_UAV:
				if (rootsig.mParameters[i].mTable)
				{
					mDx12CommandList->SetGraphicsRootDescriptorTable(i, rootsig.mParameters[i].mResource->mUAV.Gpu);
				}
				else
					mDx12CommandList->SetGraphicsRootUnorderedAccessView(i, rootsig.mParameters[i].mResource->GpuAddress);
				break;
			case PARAMETERTYPE_SAMPLER:
				mDx12CommandList->SetGraphicsRootDescriptorTable(i, rootsig.mParameters[i].mSampler->mSampler.Gpu);
				break;
			default:
				break;

			}
		}
	}

}
void CommandList::bindGraphicsResource(UINT rootindex, Resource& res)
{
	if (mCurrentBindGraphicsRootSig == nullptr)
	{
		cout << "Need to bind Graphics Root Signature First" << endl;
	}

	RootSignature& rootsig = *mCurrentBindGraphicsRootSig;


	switch (rootsig.mParameters[rootindex].mType)
	{
	case PARAMETERTYPE_ROOTCONSTANT:

		break;
	case PARAMETERTYPE_CBV:
		if (rootsig.mParameters[rootindex].mTable)
		{
			mDx12CommandList->SetGraphicsRootDescriptorTable(rootindex, res.mCBV.Gpu);
		}
		else
			mDx12CommandList->SetGraphicsRootConstantBufferView(rootindex, res.GpuAddress);
		break;
	case PARAMETERTYPE_SRV:
		if (rootsig.mParameters[rootindex].mTable)
		{
			mDx12CommandList->SetGraphicsRootDescriptorTable(rootindex, res.mSRV.Gpu);
		}
		else
			mDx12CommandList->SetGraphicsRootShaderResourceView(rootindex, res.GpuAddress);
		break;
	case PARAMETERTYPE_UAV:
		if (rootsig.mParameters[rootindex].mTable)
		{
			mDx12CommandList->SetGraphicsRootDescriptorTable(rootindex, res.mUAV.Gpu);
		}
		else
			mDx12CommandList->SetGraphicsRootUnorderedAccessView(rootindex, res.GpuAddress);
		break;
	default:
		break;

	}



}

void CommandList::bindGraphicsConstant(UINT rootindex, void const * ConstData)
{
	if (mCurrentBindGraphicsRootSig == nullptr)
	{
		cout << "Need to bind Graphics Root Signature First" << endl;
	}

	RootSignature& rootsig = *mCurrentBindGraphicsRootSig;
	mDx12CommandList->SetGraphicsRoot32BitConstants(rootindex, rootsig.mParameters[rootindex].mResCounts, ConstData, 0);
}
void CommandList::bindComputeConstant(UINT rootindex, void const * ConstData)
{
	if (mCurrentBindComputeRootSig == nullptr)
	{
		cout << "Need to bind Compute Root Signature First" << endl;
	}

	RootSignature& rootsig = *mCurrentBindComputeRootSig;
	mDx12CommandList->SetComputeRoot32BitConstants(rootindex, rootsig.mParameters[rootindex].mResCounts, ConstData, 0);
}


void CommandList::bindComputeRootSigature(RootSignature& rootsig, bool bindresource)
{
	mDx12CommandList->SetComputeRootSignature(rootsig.mRootSignature);
	mCurrentBindComputeRootSig = &rootsig;



	if (bindresource)
	{

		for (int i = 0; i < rootsig.mParameters.size(); ++i)
		{
			if (rootsig.mParameters[i].mResource == nullptr)
				continue;
			switch (rootsig.mParameters[i].mType)
			{
			case PARAMETERTYPE_ROOTCONSTANT:
				mDx12CommandList->SetComputeRoot32BitConstants(i, rootsig.mParameters[i].mResCounts, rootsig.mParameters[i].mConstantData, 0);
				break;
			case PARAMETERTYPE_CBV:
				if (rootsig.mParameters[i].mTable)
				{
					mDx12CommandList->SetComputeRootDescriptorTable(i, rootsig.mParameters[i].mResource->mCBV.Gpu);
				}
				else
					mDx12CommandList->SetComputeRootConstantBufferView(i, rootsig.mParameters[i].mResource->GpuAddress);
				break;
			case PARAMETERTYPE_SRV:
				if (rootsig.mParameters[i].mTable)
				{
					mDx12CommandList->SetComputeRootDescriptorTable(i, rootsig.mParameters[i].mResource->mSRV.Gpu);
				}
				else
					mDx12CommandList->SetComputeRootShaderResourceView(i, rootsig.mParameters[i].mResource->GpuAddress);
				break;
			case PARAMETERTYPE_UAV:
				if (rootsig.mParameters[i].mTable)
				{
					mDx12CommandList->SetComputeRootDescriptorTable(i, rootsig.mParameters[i].mResource->mUAV.Gpu);
				}
				else
					mDx12CommandList->SetComputeRootUnorderedAccessView(i, rootsig.mParameters[i].mResource->GpuAddress);
				break;
			case PARAMETERTYPE_SAMPLER:
				mDx12CommandList->SetComputeRootDescriptorTable(i, rootsig.mParameters[i].mSampler->mSampler.Gpu);
				break;
			default:
				break;

			}
		}
	}
}
void CommandList::bindComputeResource(UINT rootindex, Resource& res)
{
	if(mCurrentBindComputeRootSig==nullptr)
	{
		cout << "Need to bind Compute Root Signature First" << endl;
	}

	RootSignature& rootsig = *mCurrentBindComputeRootSig;
	switch (rootsig.mParameters[rootindex].mType)
	{
	case PARAMETERTYPE_ROOTCONSTANT:
		break;
	case PARAMETERTYPE_CBV:
		if (rootsig.mParameters[rootindex].mTable)
		{
			mDx12CommandList->SetComputeRootDescriptorTable(rootindex, res.mCBV.Gpu);
		}
		else
			mDx12CommandList->SetComputeRootConstantBufferView(rootindex, res.GpuAddress);
		break;
	case PARAMETERTYPE_SRV:
		if (rootsig.mParameters[rootindex].mTable)
		{
			mDx12CommandList->SetComputeRootDescriptorTable(rootindex, res.mSRV.Gpu);
		}
		else
			mDx12CommandList->SetComputeRootShaderResourceView(rootindex, res.GpuAddress);
		break;
	case PARAMETERTYPE_UAV:
		if (rootsig.mParameters[rootindex].mTable)
		{
			mDx12CommandList->SetComputeRootDescriptorTable(rootindex, res.mUAV.Gpu);
		}
		else
			mDx12CommandList->SetComputeRootUnorderedAccessView(rootindex, res.GpuAddress);
		break;
	default:
		break;

	}
}

void CommandList::bindVertexBuffer(Buffer& buffer)
{
	mDx12CommandList->IASetVertexBuffers(0, 1, &buffer.mVertexBuffer);
}
void CommandList::bindVertexBuffers(UINT num, Buffer const * buffer)
{
	D3D12_VERTEX_BUFFER_VIEW  vbs[D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	for (UINT i = 0; i < num; ++i)
		vbs[i] = buffer[i].mVertexBuffer;
	mDx12CommandList->IASetVertexBuffers(0, num, vbs);
}

void CommandList::bindIndexBuffer(Buffer& buffer)
{
	mDx12CommandList->IASetIndexBuffer(&buffer.mIndexBuffer);
}
void CommandList::bindDescriptorHeaps(DescriptorHeap* desheap1, DescriptorHeap* desheap2)
{
	UINT num = 1;
	ID3D12DescriptorHeap* heaps[2];
	heaps[0] = desheap1->mDescHeap;
	if (desheap2)
	{
		heaps[1] = desheap2->mDescHeap;
		num = 2;
	}
	mDx12CommandList->SetDescriptorHeaps(num, heaps);
}
void CommandList::drawInstance(UINT vertexcount, UINT instancecount, UINT vertexstart, UINT instancestart)
{
	mDx12CommandList->DrawInstanced(vertexcount, instancecount, vertexstart, instancestart);
}

void CommandList::drawIndexedInstanced(UINT indexcount, UINT instancecount, UINT indexstart, UINT instancestart, UINT vertexstart)
{
	mDx12CommandList->DrawIndexedInstanced(indexcount, instancecount, indexstart, vertexstart, instancestart);
}

void CommandList::dispatch(UINT groupCountX, UINT groupCountY, UINT groupCountZ)
{
	mDx12CommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void CommandList::executeIndirect(CommandSignature &commandsig, UINT commandcount, Buffer &commandbuffer, unsigned long long offset)
{
	mDx12CommandList->ExecuteIndirect(commandsig.mCommandSignature, commandcount, commandbuffer.mResource, offset, nullptr, 0);
}
void CommandList::executeIndirect(CommandSignature &commandsig, UINT commandcount, Buffer &commandbuffer, UINT offset, Buffer &countbuffer, UINT countoffset)
{
	mDx12CommandList->ExecuteIndirect(commandsig.mCommandSignature, commandcount, commandbuffer.mResource, offset, countbuffer.mResource, countoffset);
}

void CommandList::beginQuery(QueryHeap& queryheap, D3D12_QUERY_TYPE type, UINT index)
{
	mDx12CommandList->BeginQuery(queryheap.mHeap, type, index);
}
void CommandList::endQuery(QueryHeap& queryheap, D3D12_QUERY_TYPE type, UINT index)
{
	mDx12CommandList->EndQuery(queryheap.mHeap, type, index);
}
void CommandList::resolveQuery(QueryHeap& queryheap, D3D12_QUERY_TYPE type, UINT start, UINT numquery, Buffer resbuf, UINT64 offset)
{
	mDx12CommandList->ResolveQueryData(queryheap.mHeap, type, start, numquery, resbuf.mResource, offset);
}

void CommandList::setPrediction(Buffer& res, UINT64 offset, D3D12_PREDICATION_OP type)
{
	mDx12CommandList->SetPredication(res.mResource, offset, type);
}
void CommandList::unsetPrediction(D3D12_PREDICATION_OP type)
{
	mDx12CommandList->SetPredication(nullptr, 0, type);
}