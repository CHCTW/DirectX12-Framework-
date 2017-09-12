#include "CubeRenderTarget.h"
#include "StructureHeaders.h"
void CubeRenderTarget::release()
{
	if (mType & (CUBE_RENDERTAERGET_TYPE_DEPTH))
		mDepthBuffer[0].release();
	if (mType & (CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
		mRenderBuffer[0].release();

}

bool CubeRenderTarget::createCubeRenderTargets(ID3D12Device* device, UINT width, UINT height, CubeRenderTargetType type, DescriptorHeap& heap, DescriptorHeap& srvuavheap, D3D12_RESOURCE_FLAGS flag, DXGI_FORMAT renderforamt, DXGI_FORMAT depthforamt)
{
	if (type == (CUBE_RENDERTAERGET_TYPE_DEPTH | CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
	{
		cout << "Create Render Cube with Depth Cube need RTV and DSV heap" << endl;
		return false;
	}
	else if (type == (CUBE_RENDERTAERGET_TYPE_DEPTH))
	{
		mDepthBuffer.resize(1);
		mDepthStencilClearValue.DepthStencil.Depth = 1.0f;
		mDepthBuffer[0].CreateTexture(device, depthforamt, width, height, 1, true, flag| D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, mDepthStencilClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		mDepthBuffer[0].addSahderResorceView(srvuavheap);


		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = depthforamt;
		if (depthStencilDesc.Format == DXGI_FORMAT_R32_TYPELESS)
			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		depthStencilDesc.Texture2DArray.MipSlice = 0;
		depthStencilDesc.Texture2DArray.ArraySize = 1;
		for (int i = 0; i < 6; i++)
		{
			depthStencilDesc.Texture2DArray.FirstArraySlice = i;
			mFaceDSV[i] = heap.addResource(DSV, mDepthBuffer[0].mResource, &depthStencilDesc,nullptr);
		}
	}
	else
	{
		mRenderBuffer.resize(1);
		mRenderTargetClearValue = { 0.0,0.0,0.0,0.0 };
		mRenderBuffer[0].CreateTexture(device, renderforamt, width, height, 1, true, flag | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, mRenderTargetClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//mRenderBuffers[0].addRenderTargetView(heap);
		mRenderBuffer[0].addSahderResorceView(srvuavheap);
		if (flag & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
			mRenderBuffer[0].addUnorderedAccessView(srvuavheap);
		D3D12_RENDER_TARGET_VIEW_DESC renDesc = {};
		renDesc.Format = renderforamt;
		renDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		renDesc.Texture2DArray.MipSlice = 0;
		renDesc.Texture2DArray.PlaneSlice = 0;
		renDesc.Texture2DArray.ArraySize = 1;
		for (int i = 0; i < 6; i++)
		{
			renDesc.Texture2DArray.FirstArraySlice = i;
			mFaceRTV[i] = heap.addResource(RTV, mRenderBuffer[0].mResource, &renDesc,nullptr);
		}


	}
	mRenderTargetFormat = renderforamt;
	mDepthFormat = depthforamt;
	mWidth = width;
	mHeight = height;
	mType = type;
	return true;
}
bool CubeRenderTarget::createCubeRenderTargets(ID3D12Device* device, UINT width, UINT height, CubeRenderTargetType type, DescriptorHeap& rtvheap, DescriptorHeap& dsvheap, DescriptorHeap& srvuavheap, D3D12_RESOURCE_FLAGS renderflag, D3D12_RESOURCE_FLAGS depthflag, DXGI_FORMAT renderforamt , DXGI_FORMAT depthforamt )
{
	if (type & (CUBE_RENDERTAERGET_TYPE_DEPTH))
	{
		mDepthBuffer.resize(1);
		mDepthStencilClearValue.DepthStencil.Depth = 1.0f;
		mDepthBuffer[0].CreateTexture(device, depthforamt, width, height, 1, true, depthflag | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, mDepthStencilClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		mDepthBuffer[0].addSahderResorceView(srvuavheap);


		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = depthforamt;
		if (depthStencilDesc.Format == DXGI_FORMAT_R32_TYPELESS)
			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		depthStencilDesc.Texture2DArray.MipSlice = 0;
		depthStencilDesc.Texture2DArray.ArraySize = 1;
		for (int i = 0; i < 6; i++)
		{
			depthStencilDesc.Texture2DArray.FirstArraySlice = i;
			mFaceDSV[i] = dsvheap.addResource(DSV, mDepthBuffer[0].mResource, &depthStencilDesc,nullptr);
		}
	}
	if (type & (CUBE_RENDERTAERGET_TYPE_RENDERTARGET))
	{
		mRenderBuffer.resize(1);
		mRenderTargetClearValue = { 0.0,0.0,0.0,0.0 };
		mRenderBuffer[0].CreateTexture(device, renderforamt, width, height, 1, true, renderflag | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, mRenderTargetClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//mRenderBuffers[0].addRenderTargetView(srvuavheap);
		mRenderBuffer[0].addSahderResorceView(srvuavheap);
		if (renderflag & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
			mRenderBuffer[0].addUnorderedAccessView(srvuavheap);
		D3D12_RENDER_TARGET_VIEW_DESC renDesc = {};
		renDesc.Format = renderforamt;
		renDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		renDesc.Texture2DArray.MipSlice = 0;
		renDesc.Texture2DArray.PlaneSlice = 0;
		renDesc.Texture2DArray.ArraySize = 1;
		for (int i = 0; i < 6; i++)
		{
			renDesc.Texture2DArray.FirstArraySlice = i;
			mFaceRTV[i] = rtvheap.addResource(RTV, mRenderBuffer[0].mResource, &renDesc, nullptr);
		}


	}
	mRenderTargetFormat = renderforamt;
	mDepthFormat = depthforamt;
	mWidth = width;
	mHeight = height;
	mType = type;
	return true;
}
