//#include "CubeRenderTarget.h"
//#include "StructureHeaders.h"
//void CubeRenderTarget::release()
//{
//	if (mType & (CUBE_RENDERTAERGET_TYPE_DEPTH))
//		mDepthBuffer[0].release();
//	if (mType & (CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
//		mRenderBuffer[0].release();
//
//}
//
//bool CubeRenderTarget::createCubeRenderTargets(ID3D12Device* device, UINT width, UINT height, UINT arraysize, UINT miplevel, CubeRenderTargetType type, DescriptorHeap& heap, DescriptorHeap& srvuavheap, D3D12_RESOURCE_FLAGS flag, DXGI_FORMAT renderforamt, DXGI_FORMAT depthforamt)
//{
//	if (type == (CUBE_RENDERTAERGET_TYPE_DEPTH | CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
//	{
//		cout << "Create Render Cube with Depth Cube need RTV and DSV heap" << endl;
//		return false;
//	}
//	else if (type == (CUBE_RENDERTAERGET_TYPE_DEPTH))
//	{
//		mDepthBuffer.resize(1);
//		mDepthStencilClearValue.DepthStencil.Depth = 1.0f;
//		mDepthBuffer[0].CreateTexture(device, depthforamt, width, height, arraysize, true, miplevel, flag| D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, mDepthStencilClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_DEPTH_WRITE);
//		mDepthBuffer[0].addSahderResorceView(srvuavheap);
//
//
//		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
//		depthStencilDesc.Format = depthforamt;
//		if (depthStencilDesc.Format == DXGI_FORMAT_R32_TYPELESS)
//			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
//		if (depthStencilDesc.Format == DXGI_FORMAT_R16_TYPELESS)
//			depthStencilDesc.Format = DXGI_FORMAT_D16_UNORM;
//		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
//		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
//		depthStencilDesc.Texture2DArray.FirstArraySlice = 0;
//		depthStencilDesc.Texture2DArray.ArraySize = 6 * arraysize;
//		mFaceDSV.resize(miplevel);
//		for (int j = 0; j < miplevel; ++j)
//		{
//			depthStencilDesc.Texture2DArray.MipSlice = j;
//			mFaceDSV[j] = heap.addResource(DSV, mDepthBuffer[0].mResource, &depthStencilDesc, nullptr);
//		}
//		
//	}
//	else
//	{
//		mRenderBuffer.resize(1);
//		mRenderTargetClearValue.setColor(0.0,0.0,0.0,0.0 );
//		mRenderBuffer[0].CreateTexture(device, renderforamt, width, height, arraysize, true, miplevel, flag | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, mRenderTargetClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D,  D3D12_RESOURCE_STATE_RENDER_TARGET);
//		//mRenderBuffers[0].addRenderTargetView(heap);
//		mRenderBuffer[0].addSahderResorceView(srvuavheap);
//		if (flag & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
//			mRenderBuffer[0].addUnorderedAccessView(srvuavheap);
//		D3D12_RENDER_TARGET_VIEW_DESC renDesc = {};
//		renDesc.Format = renderforamt;
//		renDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
//		renDesc.Texture2DArray.FirstArraySlice = 0;
//		renDesc.Texture2DArray.ArraySize = 6 * arraysize;
//		mFaceRTV.resize(miplevel);
//		for (int j = 0; j < miplevel; ++j)
//		{
//			renDesc.Texture2DArray.MipSlice = j;
//			mFaceRTV[j] = heap.addResource(RTV, mRenderBuffer[0].mResource, &renDesc, nullptr);
//		}
//		
//
//
//	}
//	mRenderTargetFormat = renderforamt;
//	mDepthFormat = depthforamt;
//	mWidth = width;
//	mHeight = height;
//	mArraySize = arraysize;
//	mType = type;
//	return true;
//}
//bool CubeRenderTarget::createCubeRenderTargets(ID3D12Device* device, UINT width, UINT height, UINT arraysize, UINT miplevel, CubeRenderTargetType type, DescriptorHeap& rtvheap, DescriptorHeap& dsvheap, DescriptorHeap& srvuavheap, D3D12_RESOURCE_FLAGS renderflag, D3D12_RESOURCE_FLAGS depthflag, DXGI_FORMAT renderforamt , DXGI_FORMAT depthforamt )
//{
//	if (type & (CUBE_RENDERTAERGET_TYPE_DEPTH))
//	{
//		mDepthBuffer.resize(1);
//		mDepthStencilClearValue.DepthStencil.Depth = 1.0f;
//		mDepthBuffer[0].CreateTexture(device, depthforamt, width, height, arraysize, true, miplevel, depthflag | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, mDepthStencilClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_DEPTH_WRITE);
//		mDepthBuffer[0].addSahderResorceView(srvuavheap);
//
//
//		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
//		depthStencilDesc.Format = depthforamt;
//		if (depthStencilDesc.Format == DXGI_FORMAT_R32_TYPELESS)
//			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
//		if (depthStencilDesc.Format == DXGI_FORMAT_R16_TYPELESS)
//			depthStencilDesc.Format = DXGI_FORMAT_D16_UNORM;
//		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
//		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
//		depthStencilDesc.Texture2DArray.MipSlice = 0;
//		depthStencilDesc.Texture2DArray.ArraySize = 6* arraysize;
//		mFaceDSV.resize(miplevel);
//		for (int j = 0; j < miplevel; ++j)
//		{
//			depthStencilDesc.Texture2DArray.MipSlice = j;
//			mFaceDSV[j] = dsvheap.addResource(DSV, mDepthBuffer[0].mResource, &depthStencilDesc, nullptr);
//		}
//		
//	}
//	if (type & (CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
//	{
//		mRenderBuffer.resize(1);
//		mRenderTargetClearValue.setColor(0.0,0.0,0.0,0.0);
//		mRenderBuffer[0].CreateTexture(device, renderforamt, width, height, arraysize, true, miplevel, renderflag | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, mRenderTargetClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_RENDER_TARGET);
//		//mRenderBuffers[0].addRenderTargetView(srvuavheap);
//		mRenderBuffer[0].addSahderResorceView(srvuavheap);
//		if (renderflag & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
//			mRenderBuffer[0].addUnorderedAccessView(srvuavheap);
//		D3D12_RENDER_TARGET_VIEW_DESC renDesc = {};
//		renDesc.Format = renderforamt;
//		renDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
//		renDesc.Texture2DArray.PlaneSlice = 0;
//		renDesc.Texture2DArray.ArraySize = 6 * arraysize;
//		
//		mFaceRTV.resize(miplevel);
//		for (int j = 0; j < miplevel; ++j)
//		{
//			renDesc.Texture2DArray.MipSlice = j;
//			mFaceRTV[j] = rtvheap.addResource(RTV, mRenderBuffer[0].mResource, &renDesc, nullptr);
//		}
//		
//
//
//	}
//	mRenderTargetFormat = renderforamt;
//	mDepthFormat = depthforamt;
//	mWidth = width;
//	mHeight = height;
//	mArraySize = arraysize;
//	mType = type;
//	return true;
//}
