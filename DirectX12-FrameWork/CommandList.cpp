#include "stdafx.h"
#include "StructureHeaders.h"
CommandList::CommandList():mDx12CommandList(nullptr), mDx12Allocater(nullptr), mCurrentBindGraphicsRootSig(nullptr), mCurrentBindComputeRootSig(nullptr)
{
	mAccuBarriers.reserve(16); //ask for 16 first
}
bool CommandList::initial(ID3D12Device* device, CommandAllocator &alloc)
{
	HRESULT hr;

	mType = alloc.mType;
	switch (mType)
	{
	case COMMAND_TYPE_GRAPHICS:
		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.mDx12Allocater, NULL, IID_PPV_ARGS(&mDx12CommandList));
		break;
	case COMMAND_TYPE_COMPUTE:
		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, alloc.mDx12Allocater, NULL, IID_PPV_ARGS(&mDx12CommandList));
		break;
	case COMMAND_TYPE_COPY:
		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, alloc.mDx12Allocater, NULL, IID_PPV_ARGS(&mDx12CommandList));
		break;
	}


	
	if (FAILED(hr))
		return false;
	
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
// use with causion, doesn't provide any allocator check yet, this is a pre step for command list pool
bool CommandList::resetwithAllocate(Pipeline& pipeline)
{
	HRESULT hr;
	hr = mDx12Allocater->Reset();
	if (FAILED(hr))
		return false;
	hr = mDx12CommandList->Reset(mDx12Allocater, pipeline.mPipeline);
	if (FAILED(hr))
		return false;
	return true;
}
bool CommandList::resetwithAllocate()
{
	HRESULT hr;
	hr = mDx12Allocater->Reset();
	if (FAILED(hr))
		return false;
	hr = mDx12CommandList->Reset(mDx12Allocater, nullptr);
	if (FAILED(hr))
		return false;
	return true;
}

void CommandList::bindPipeline(Pipeline& pipeline)
{
	mDx12CommandList->SetPipelineState(pipeline.mPipeline);
}
//void CommandList::resourceBarrier(Resource& res, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
//	D3D12_RESOURCE_BARRIER_FLAGS flags)
//{
//	
//	mDx12CommandList->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(res.mResource,statbef,stataf,subresource,flags));
//	if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
//		std::fill(res.mState.begin(), res.mState.end(), stataf);
//	else
//		res.mState[subresource] = stataf;
//}
void CommandList::UAVWait(Resource& res, bool barrier)
{
	D3D12_RESOURCE_BARRIER wait;
	wait.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	wait.UAV.pResource = res.mResource;
	wait.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	mAccuBarriers.push_back(wait); // add to current
	if (barrier) // if set barrier
	{
		setBarrier();
	}

}
void CommandList::resourceTransition(Resource& res, D3D12_RESOURCE_STATES stataf, bool barrier, UINT subresource,D3D12_RESOURCE_BARRIER_FLAGS flags)
{
	if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		if (res.mState[0] == stataf)   // same state no transition 
			return;
	}
	else
	{
		if (res.mState[subresource] == stataf)
			return;
	}

	D3D12_RESOURCE_BARRIER transition;
	transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	transition.Flags = flags;
	transition.Transition.pResource = res.mResource;
	transition.Transition.StateAfter = stataf;
	transition.Transition.Subresource = subresource;
	if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		
		transition.Transition.StateBefore = res.mState[0];// all subresource transition, doesn't matter which one..., what if not all subresource in the same state?
		std::fill(res.mState.begin(), res.mState.end(), stataf);
	}
	else
	{
		transition.Transition.StateBefore = res.mState[subresource];
		res.mState[subresource] = stataf;
	}
	mAccuBarriers.push_back(transition); // add to current
	if (barrier) // if set barrier
	{
		setBarrier();
	}

}
void CommandList::swapChainBufferTransition(SwapChainBuffer& res, D3D12_RESOURCE_STATES stataf, bool barrier,
	D3D12_RESOURCE_BARRIER_FLAGS flags)
{
	if (res.mState[0] != stataf)   // same state no transition 
	{
		D3D12_RESOURCE_BARRIER transition;
		transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition.Flags = flags;
		transition.Transition.pResource = res.mResource;
		transition.Transition.StateAfter = stataf;
		transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		transition.Transition.StateBefore = res.mState[0];// all subresource transition, doesn't matter which one..., what if not all subresource in the same state?
		res.mState[0] = stataf;
		mAccuBarriers.push_back(transition); // add to current
	}
	if (barrier) // if set barrier
	{
		setBarrier();
	}

}



void CommandList::setBarrier()
{
	if(mAccuBarriers.size()>0)
		mDx12CommandList->ResourceBarrier(mAccuBarriers.size(), mAccuBarriers.data());
	mAccuBarriers.clear();
}

//void CommandList::resourceBarrier(ID3D12Resource* res, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
//	D3D12_RESOURCE_BARRIER_FLAGS flags)
//{
//
//	mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res, statbef, stataf, subresource, flags));
//}
//void CommandList::renderTargetBarrier(RenderTarget &rt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
//	D3D12_RESOURCE_BARRIER_FLAGS flags)
//{
//
//	
//	for (int i = 0; i < rt.mRenderBuffers.size(); ++i)
//	{
//		mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rt.mRenderBuffers[i].mResource, statbef, stataf, subresource, flags));
//	}
//	//mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rt.mRenderBuffer, statbef, stataf, subresource, flags));
//	//rt.mRenderBufferState = stataf;
//}
//void CommandList::depthBufferBarrier(RenderTarget &rt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,D3D12_RESOURCE_BARRIER_FLAGS flags)
//{
//	if (rt.mDepthBuffer.size() == 1)
//	{
//		mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rt.mDepthBuffer[0].mResource, statbef, stataf, subresource, flags));
//	}
//}
//
//
//void CommandList::cubeRenderTargetBarrier(CubeRenderTarget &crt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
//	D3D12_RESOURCE_BARRIER_FLAGS flags)
//{
//	if (crt.mType & (CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
//	{
//		mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(crt.mRenderBuffer[0].mResource, statbef, stataf, subresource, flags));
//	}
//}
//void CommandList::cubeDepthBufferBarrier(CubeRenderTarget &crt, D3D12_RESOURCE_STATES statbef, D3D12_RESOURCE_STATES stataf, UINT subresource,
//	D3D12_RESOURCE_BARRIER_FLAGS flags)
//{
//	if (crt.mType & (CUBE_RENDERTAERGET_TYPE_DEPTH))
//	{
//		mDx12CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(crt.mDepthBuffer[0].mResource, statbef, stataf, subresource, flags));
//	}
//}


bool CommandList::close()
{
	HRESULT hr;
	hr = mDx12CommandList->Close();
	if (FAILED(hr))
		return false;
	return true;
}
//void CommandList::bindRenderTarget(RenderTarget & rt, UINT miplevel)
//{
//	CD3DX12_CPU_DESCRIPTOR_HANDLE cpus[8] = {};
//	for (int i = 0 ; i < rt.mRenderBuffers.size() ; ++i)
//	{
//		cpus[i] = rt.mRenderBuffers[i].mRTV[miplevel].Cpu;
//	}
//	if(rt.mFormat.mDepth)
//		mDx12CommandList->OMSetRenderTargets(rt.mRenderBuffers.size(), cpus, false, &(rt.mDepthBuffer[0].mDSV[miplevel].Cpu));
//	else
//	mDx12CommandList->OMSetRenderTargets(rt.mRenderBuffers.size(), cpus, false, nullptr);
//}
void CommandList::bindRenderTarget(SwapChainBuffer & rt)
{
	mDx12CommandList->OMSetRenderTargets(1, &rt.mRTV[0].Cpu, false, nullptr);
}
void CommandList::bindRenderTarget(SwapChainBuffer & rt, Texture & depthstencilbuffer, UINT depthstecmip)
{
	mDx12CommandList->OMSetRenderTargets(1, &rt.mRTV[0].Cpu, false, &depthstencilbuffer.mDSV[depthstecmip].Cpu);
}

void CommandList::bindDepthStencilBufferOnly(Texture& dsbuffer, UINT miplevel)
{
	mDx12CommandList->OMSetRenderTargets(0, nullptr, false, &dsbuffer.mDSV[miplevel].Cpu);
}

void CommandList::bindRenderTargetsOnly(Texture& t1, UINT mip)
{
	mDx12CommandList->OMSetRenderTargets(1, &t1.mRTV[mip].Cpu, false, nullptr);
}
void CommandList::bindRenderTargetsOnly(Texture& t1, Texture& t2, UINT mip)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[2];
	rtvs[0] = t1.mRTV[mip].Cpu;
	rtvs[1] = t2.mRTV[mip].Cpu;
	mDx12CommandList->OMSetRenderTargets(2, rtvs, false, nullptr);
}
void CommandList::bindRenderTargetsOnly(Texture& t1, Texture& t2, Texture& t3, UINT mip)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[3];
	rtvs[0] = t1.mRTV[mip].Cpu;
	rtvs[1] = t2.mRTV[mip].Cpu;
	rtvs[2] = t3.mRTV[mip].Cpu;
	mDx12CommandList->OMSetRenderTargets(3, rtvs, false, nullptr);
}
void CommandList::bindRenderTargetsOnly(Texture& t1, Texture& t2, Texture& t3, Texture& t4, UINT mip)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[4];
	rtvs[0] = t1.mRTV[mip].Cpu;
	rtvs[1] = t2.mRTV[mip].Cpu;
	rtvs[2] = t3.mRTV[mip].Cpu;
	rtvs[3] = t4.mRTV[mip].Cpu;
	mDx12CommandList->OMSetRenderTargets(4, rtvs, false, nullptr);
}
void CommandList::bindRenderTargetsDepthStencil(Texture& t1, Texture& ds, UINT retmip, UINT dsmip)
{
	mDx12CommandList->OMSetRenderTargets(1, &t1.mRTV[retmip].Cpu, false, &ds.mDSV[dsmip].Cpu);
}
void CommandList::bindRenderTargetsDepthStencil(Texture& t1, Texture& t2, Texture& ds, UINT retmip, UINT dsmip)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[2];
	rtvs[0] = t1.mRTV[retmip].Cpu;
	rtvs[1] = t2.mRTV[retmip].Cpu;
	mDx12CommandList->OMSetRenderTargets(2, rtvs, false, &ds.mDSV[dsmip].Cpu);
}
void CommandList::bindRenderTargetsDepthStencil(Texture& t1, Texture& t2, Texture& t3, Texture& ds, UINT retmip, UINT dsmip)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[3];
	rtvs[0] = t1.mRTV[retmip].Cpu;
	rtvs[1] = t2.mRTV[retmip].Cpu;
	rtvs[2] = t3.mRTV[retmip].Cpu;
	mDx12CommandList->OMSetRenderTargets(3, rtvs, false, &ds.mDSV[dsmip].Cpu);
}
void CommandList::bindRenderTargetsDepthStencil(Texture& t1, Texture& t2, Texture& t3, Texture& t4, Texture& ds, UINT retmip, UINT dsmip)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[4];
	rtvs[0] = t1.mRTV[retmip].Cpu;
	rtvs[1] = t2.mRTV[retmip].Cpu;
	rtvs[2] = t3.mRTV[retmip].Cpu;
	rtvs[3] = t4.mRTV[retmip].Cpu;
	mDx12CommandList->OMSetRenderTargets(3, rtvs, false, &ds.mDSV[dsmip].Cpu);
}






//void CommandList::bindCubeRenderTarget(CubeRenderTarget & crt, UINT face,UINT level)
//{
//	if (crt.mType == (CUBE_RENDERTAERGET_TYPE_DEPTH| CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
//	{
//		mDx12CommandList->OMSetRenderTargets(1, &(crt.mFaceRTV[level].Cpu), false, &crt.mFaceDSV[level].Cpu);
//	}
//	else if (crt.mType == (CUBE_RENDERTAERGET_TYPE_DEPTH))
//	{
//		mDx12CommandList->OMSetRenderTargets(0, nullptr, false, &crt.mFaceDSV[level].Cpu);
//	}
//	else
//		mDx12CommandList->OMSetRenderTargets(1, &(crt.mFaceRTV[level].Cpu), false, nullptr);
//}
//
//void CommandList::clearRenderTarget(RenderTarget &rt, const float *color,  UINT miplevel)
//{
////	CD3DX12_CPU_DESCRIPTOR_HANDLE cpus[8] = {};
//	for (int i = 0; i < rt.mRenderBuffers.size(); ++i)
//	{
//	//	cpus[i] = rt.mRenderBuffers[i].mRTV.Cpu;
//
//		mDx12CommandList->ClearRenderTargetView(rt.mRenderBuffers[i].mRTV[miplevel].Cpu, color, 0, NULL);
//
//	}
////	mDx12CommandList->ClearRenderTargetView(rt.mRTV.Cpu, color, 0, NULL);
//}
//void CommandList::clearRenderTarget(RenderTarget &rt,UINT miplevel)
//{
//	for (int i = 0; i < rt.mRenderBuffers.size(); ++i)
//	{
//		//	cpus[i] = rt.mRenderBuffers[i].mRTV.Cpu;
//
//		mDx12CommandList->ClearRenderTargetView(rt.mRenderBuffers[i].mRTV[miplevel].Cpu, rt.mFormat.mRenderTargetClearValue[i].Color, 0, NULL);
//
//	}
//}
void CommandList::clearRenderTarget(Texture &rt, const float *color, UINT miplevel)
{
	mDx12CommandList->ClearRenderTargetView(rt.mRTV[miplevel].Cpu, color, 0, NULL);
}
void CommandList::clearRenderTarget(Texture &rt, UINT miplevel)
{
	mDx12CommandList->ClearRenderTargetView(rt.mRTV[miplevel].Cpu, rt.mClearVal.Color, 0, NULL);
}
void CommandList::clearRenderTarget(SwapChainBuffer &rt, const float *color)
{
	mDx12CommandList->ClearRenderTargetView(rt.mRTV[0].Cpu, color, 0, NULL);
}


//void CommandList::clearDepthStencil(RenderTarget &rt, D3D12_CLEAR_FLAGS flag, float depth, UINT stencil, UINT miplevel)
//{
//	if (rt.mFormat.mDepth)
//	{
//		mDx12CommandList->ClearDepthStencilView(rt.mDepthBuffer[0].mDSV[miplevel].Cpu, flag, depth, stencil, 0, nullptr);
//	}
//}
void CommandList::clearDepthStencil(Texture &dsbuffer, D3D12_CLEAR_FLAGS flag, float depth , UINT stencil, UINT miplevel)
{
	if (dsbuffer.mTextUsage&TEXTURE_USAGE_DSV)
	{
		mDx12CommandList->ClearDepthStencilView(dsbuffer.mDSV[miplevel].Cpu, flag, depth, stencil, 0, nullptr);
	}
}


//void CommandList::clearcubeRenderTarget(CubeRenderTarget &crt, UINT face, UINT level)
//{
//	if (crt.mType & (CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
//	{
//		mDx12CommandList->ClearRenderTargetView(crt.mFaceRTV[level].Cpu, crt.mRenderTargetClearValue.Color, 0, NULL);
//	}
//
//}
//void CommandList::clearcubeDepthStencil(CubeRenderTarget &crt, UINT face, UINT level, D3D12_CLEAR_FLAGS flag, float depth, UINT stencil)
//{
//	if (crt.mType & (CUBE_RENDERTAERGET_TYPE_DEPTH))
//	{
//		mDx12CommandList->ClearDepthStencilView(crt.mFaceDSV[level].Cpu, flag, depth, stencil, 0, nullptr);
//	}
//}



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
bool CommandList::updateBufferData(DynamicUploadBuffer& upload, Buffer& buffer, void  const  * data, UINT64 datasize, UINT64 bufferoffset)
{
	AllocateFormat format = upload.allocateforCurrentFrame(datasize); // get allocated buffer
	char const* copysrc = (char const*)data; 
	UINT64 srcoffset = 0;
	for (int i = 0; i < format.inform.mapcount; ++i) // may be 1 or 2 times, since it's a ring buffer, may need to copy twice
	{
		char* copydest = (char *)format.cpubuffer;// change to char* to add the offset
		copydest += format.inform.maplist[i].first; // +offset
		copysrc += srcoffset;
		memcpy(copydest, copysrc, format.inform.maplist[i].second);  // copy to upload buffer
		
		// record the command from ring buffer to dest buffer, since we already store the data in ring buffer, it is ok to update src data
		mDx12CommandList->CopyBufferRegion(buffer.mResource, bufferoffset+srcoffset, format.gpubuffer, format.inform.maplist[i].first, format.inform.maplist[i].second);
//		cout << format.inform.maplist[i].first << "    " << format.inform.maplist[i].second << endl;
		srcoffset += format.inform.maplist[i].second; // next round, we start from the remain
	}
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
bool CommandList::copyResource(Resource &src, Resource &desc)
{
	mDx12CommandList->CopyResource(desc.mResource,src.mResource);
	return true;
}
bool CommandList::updateTextureData(Texture& texture, void const * data, UINT startlevel, UINT levelnum, UINT startslice, UINT slicenum)
{
	if (!texture.mResource || !texture.mUploadBuffer)
		return false;
	UINT levels = min((texture.textureDesc.MipLevels - startlevel), levelnum);
	UINT slices = min((texture.textureDesc.DepthOrArraySize - startslice), slicenum);


	D3D12_SUBRESOURCE_DATA *Data = new D3D12_SUBRESOURCE_DATA[levels];

	UINT64 cpuoffset = 0;
	UINT64 gpuoffset = 0;
	for (unsigned int i = startslice; i < slices + startslice; ++i)
	{
		unsigned startsub = startslice*texture.textureDesc.MipLevels + startlevel;
		gpuoffset = texture.mLayouts[startsub].Offset;
		for (unsigned int j = startlevel; j < startlevel + levels; ++j)
		{
			unsigned int subnum = i*texture.textureDesc.MipLevels + j;
			unsigned int cpumiplevel = j - startlevel;
			Data[cpumiplevel].pData = (const char *)data + cpuoffset;
			Data[cpumiplevel].RowPitch = texture.mLayouts[subnum].Footprint.Width*texture.mByteSize;
			Data[cpumiplevel].SlicePitch = texture.mLayouts[subnum].Footprint.Height*Data[cpumiplevel].RowPitch;
			cpuoffset += Data[cpumiplevel].SlicePitch;
		}
		UpdateSubresources(mDx12CommandList, texture.mResource, texture.mUploadBuffer, gpuoffset, startsub, levels, Data);
	}

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
					mDx12CommandList->SetGraphicsRootDescriptorTable(i, rootsig.mParameters[i].mResource->mUAV[rootsig.mParameters[i].mUAVMipLevel].Gpu);
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
	bindGraphicsResource(rootindex,res, mCurrentBindGraphicsRootSig->mParameters[rootindex].mUAVMipLevel);
}
void CommandList::bindGraphicsResource(UINT rootindex, Resource& res, UINT uavmiplevel)
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
			mDx12CommandList->SetGraphicsRootDescriptorTable(rootindex, res.mUAV[uavmiplevel].Gpu);
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
					mDx12CommandList->SetComputeRootDescriptorTable(i, rootsig.mParameters[i].mResource->mUAV[rootsig.mParameters[i].mUAVMipLevel].Gpu);
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
	if (mCurrentBindComputeRootSig == nullptr)
	{
		cout << "Need to bind Compute Root Signature First" << endl;
	}
	bindComputeResource(rootindex,res, mCurrentBindComputeRootSig->mParameters[rootindex].mUAVMipLevel);
}
void CommandList::bindComputeResource(UINT rootindex, Resource& res,UINT uavmiplevel)
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
			mDx12CommandList->SetComputeRootDescriptorTable(rootindex, res.mUAV[uavmiplevel].Gpu);
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