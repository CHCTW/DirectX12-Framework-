//#include "RenderTarget.h"
//
//#include "StructureHeaders.h"
//bool RenderTarget::createRenderTargets(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& rtvheap, DescriptorHeap& dsvheap, DescriptorHeap& cbvsrvuavheap)
//{
//
//	mFormat = format;
//	mRenderBuffers.resize(mFormat.mNumofRenderTargets);
//	for (int i = 0; i < mFormat.mNumofRenderTargets; ++i)  // create and add views for textures 
//	{
//		mRenderBuffers[i].CreateTexture(device, mFormat.mRenderTargetFormat[i], width, height, 1, false, 1, mFormat.mRenderTargetflags, mFormat.mRenderTargetClearValue[i], D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_RENDER_TARGET);
//		mRenderBuffers[i].addRenderTargetView(rtvheap);
//		mRenderBuffers[i].addSahderResorceView(cbvsrvuavheap);
//		if (mFormat.mRenderTargetflags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
//			mRenderBuffers[i].addUnorderedAccessView(cbvsrvuavheap);
//	}
//	if (mFormat.mDepth)
//	{
//		mDepthBuffer.resize(1);
//		mDepthBuffer[0].CreateTexture(device, mFormat.mDepthStencilFormat, width, height, 1,false, 1, mFormat.mDepthStencilflags, mFormat.mDepthStencilClearValue,  D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_DEPTH_WRITE);
//		mDepthBuffer[0].addDepgthStencilView(dsvheap);
//		mDepthBuffer[0].addSahderResorceView(cbvsrvuavheap);
//		if (mFormat.mDepthStencilflags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)// haven't find the formart that can support uav+dsv
//			mDepthBuffer[0].addUnorderedAccessView(cbvsrvuavheap);
//		//mDepthBuffer[0].addUnorderedAccessView(cbvsrvuavheap);
//	}
//
//
//	return true;
//}
//bool RenderTarget::createRenderTargets(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& heap, DescriptorHeap& cbvsrvuavheap)
//{
//	if (format.mDepth && format.mNumofRenderTargets>0)
//	{
//		cout << "Create render targets with depth buffer need RTV and DSV Heap" << endl;
//		return false;
//	}
//
//	
//	mFormat = format;
//	mRenderBuffers.resize(mFormat.mNumofRenderTargets);
//	if (!format.mDepth)
//	{
//		
//		for (int i = 0; i < mFormat.mNumofRenderTargets; ++i)  // create and add views for textures 
//		{
//			mRenderBuffers[i].CreateTexture(device, mFormat.mRenderTargetFormat[i], width, height,1 ,false, 1, mFormat.mRenderTargetflags, mFormat.mRenderTargetClearValue[i], D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_RENDER_TARGET);
//			mRenderBuffers[i].addRenderTargetView(heap);
//			mRenderBuffers[i].addSahderResorceView(cbvsrvuavheap);
//			if (mFormat.mRenderTargetflags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) 
//				mRenderBuffers[i].addUnorderedAccessView(cbvsrvuavheap);
//		}
//	}
//	else
//	{
//		mDepthBuffer.resize(1);
//		mDepthBuffer[0].CreateTexture(device, mFormat.mDepthStencilFormat, width, height,1 , false, 1,  mFormat.mDepthStencilflags, mFormat.mDepthStencilClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_DEPTH_WRITE);
//		mDepthBuffer[0].addDepgthStencilView(heap);
//		mDepthBuffer[0].addSahderResorceView(cbvsrvuavheap);
//		if (mFormat.mDepthStencilflags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) // haven't find the formart that can support uav+dsv
//			mDepthBuffer[0].addUnorderedAccessView(cbvsrvuavheap);
//	}
//
//
//	return true;
//}
////bool RenderTarget::createCubeDepthBuffers(ID3D12Device* device, UINT width, UINT height, RenderTargetFormat& format, DescriptorHeap& heap, DescriptorHeap& cbvsrvuavheap)
////{
////	return true;
////}
//
//void RenderTarget::release()
//{
//	for (auto &ren : mRenderBuffers)
//	{
//		ren.release();
//	}
//	for (auto &dep : mDepthBuffer)
//	{
//		dep.release();
//	}
//
//}