#include "Texture.h"
#include "stdafx.h"
#include "StructureHeaders.h"
UINT getSize(DXGI_FORMAT format)
{


	UINT size = 0;
	switch (format)
	{
	case DXGI_FORMAT_UNKNOWN:
		break;
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		size = 16;
		break;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		size = 16;
		break;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		size = 16;
		break;
	case DXGI_FORMAT_R32G32B32A32_SINT:
		size = 16;
		break;
	case DXGI_FORMAT_R32G32B32_TYPELESS:
		size = 16;
		break;
	case DXGI_FORMAT_R32G32B32_FLOAT:
		size = 16;
		break;
	case DXGI_FORMAT_R32G32B32_UINT:
		size = 12;
		break;
	case DXGI_FORMAT_R32G32B32_SINT:
		size = 12;
		break;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		break;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		break;
	case DXGI_FORMAT_R16G16B16A16_UINT:
		break;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		break;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		break;
	case DXGI_FORMAT_R32G32_TYPELESS:
		size = 8;
		break;
	case DXGI_FORMAT_R32G32_FLOAT:
		size = 8;
		break;
	case DXGI_FORMAT_R32G32_UINT:
		size = 8;
		break;
	case DXGI_FORMAT_R32G32_SINT:
		size = 8;
		break;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		break;
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		break;
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		break;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		break;
	case DXGI_FORMAT_R10G10B10A2_UINT:
		break;
	case DXGI_FORMAT_R11G11B10_FLOAT:
		break;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		size = 4;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		size = 4;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		size = 4;
		break;
	case DXGI_FORMAT_R8G8B8A8_UINT:
		size = 4;
		break;
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		size = 4;
		break;
	case DXGI_FORMAT_R8G8B8A8_SINT:
		size = 4;
		break;
	case DXGI_FORMAT_R16G16_TYPELESS:
		size = 4;
		break;
	case DXGI_FORMAT_R16G16_FLOAT:
		size = 4;
		break;
	case DXGI_FORMAT_R16G16_UNORM:
		size = 4;
		break;
	case DXGI_FORMAT_R16G16_UINT:
		size = 4;
		break;
	case DXGI_FORMAT_R16G16_SNORM:
		size = 4;
		break;
	case DXGI_FORMAT_R16G16_SINT:
		size = 4;
		break;
	case DXGI_FORMAT_R32_TYPELESS:
		size = 4;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		size =  4;
		break;
	case DXGI_FORMAT_R32_FLOAT:
		break;
	case DXGI_FORMAT_R32_UINT:
		break;
	case DXGI_FORMAT_R32_SINT:
		break;
	case DXGI_FORMAT_R24G8_TYPELESS:
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		break;
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		break;
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		break;
	case DXGI_FORMAT_R8G8_TYPELESS:
		break;
	case DXGI_FORMAT_R8G8_UNORM:
		break;
	case DXGI_FORMAT_R8G8_UINT:
		break;
	case DXGI_FORMAT_R8G8_SNORM:
		break;
	case DXGI_FORMAT_R8G8_SINT:
		size = 2;
		break;
	case DXGI_FORMAT_R16_TYPELESS:
		size = 2;
		break;
	case DXGI_FORMAT_R16_FLOAT:
		size = 2;
		break;
	case DXGI_FORMAT_D16_UNORM:
		size = 2;
		break;
	case DXGI_FORMAT_R16_UNORM:
		break;
	case DXGI_FORMAT_R16_UINT:
		break;
	case DXGI_FORMAT_R16_SNORM:
		break;
	case DXGI_FORMAT_R16_SINT:
		break;
	case DXGI_FORMAT_R8_TYPELESS:
		break;
	case DXGI_FORMAT_R8_UNORM:
		break;
	case DXGI_FORMAT_R8_UINT:
		break;
	case DXGI_FORMAT_R8_SNORM:
		break;
	case DXGI_FORMAT_R8_SINT:
		break;
	case DXGI_FORMAT_A8_UNORM:
		break;
	case DXGI_FORMAT_R1_UNORM:
		break;
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		break;
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
		break;
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
		break;
	case DXGI_FORMAT_BC1_TYPELESS:
		break;
	case DXGI_FORMAT_BC1_UNORM:
		break;
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		break;
	case DXGI_FORMAT_BC2_TYPELESS:
		break;
	case DXGI_FORMAT_BC2_UNORM:
		break;
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		break;
	case DXGI_FORMAT_BC3_TYPELESS:
		break;
	case DXGI_FORMAT_BC3_UNORM:
		break;
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		break;
	case DXGI_FORMAT_BC4_TYPELESS:
		break;
	case DXGI_FORMAT_BC4_UNORM:
		break;
	case DXGI_FORMAT_BC4_SNORM:
		break;
	case DXGI_FORMAT_BC5_TYPELESS:
		break;
	case DXGI_FORMAT_BC5_UNORM:
		break;
	case DXGI_FORMAT_BC5_SNORM:
		break;
	case DXGI_FORMAT_B5G6R5_UNORM:
		break;
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		break;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		size = 4;
		break;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		size = 4;
		break;
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		break;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		size = 4;
		break;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		break;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		break;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		break;
	case DXGI_FORMAT_BC6H_TYPELESS:
		break;
	case DXGI_FORMAT_BC6H_UF16:
		break;
	case DXGI_FORMAT_BC6H_SF16:
		break;
	case DXGI_FORMAT_BC7_TYPELESS:
		break;
	case DXGI_FORMAT_BC7_UNORM:
		break;
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		break;
	case DXGI_FORMAT_AYUV:
		break;
	case DXGI_FORMAT_Y410:
		break;
	case DXGI_FORMAT_Y416:
		break;
	case DXGI_FORMAT_NV12:
		break;
	case DXGI_FORMAT_P010:
		break;
	case DXGI_FORMAT_P016:
		break;
	case DXGI_FORMAT_420_OPAQUE:
		break;
	case DXGI_FORMAT_YUY2:
		break;
	case DXGI_FORMAT_Y210:
		break;
	case DXGI_FORMAT_Y216:
		break;
	case DXGI_FORMAT_NV11:
		break;
	case DXGI_FORMAT_AI44:
		break;
	case DXGI_FORMAT_IA44:
		break;
	case DXGI_FORMAT_P8:
		break;
	case DXGI_FORMAT_A8P8:
		break;
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		break;
	case DXGI_FORMAT_P208:
		break;
	case DXGI_FORMAT_V208:
		break;
	case DXGI_FORMAT_V408:
		break;
	case DXGI_FORMAT_FORCE_UINT:
		break;
	default:

		cout << "Undefined Format, Need to add it" << format << endl;
		break;
	}
	return size;
}


Texture::Texture(): mByteSize(0), mCubeMap(false)
{
}


void Texture::CreateTexture(ID3D12Device* device, DXGI_FORMAT format, UINT width, UINT height, UINT arraySize, bool isCubeMap, UINT mipLevel, D3D12_RESOURCE_FLAGS usage, ClearValue& clear, D3D12_RESOURCE_DIMENSION dem, D3D12_RESOURCE_STATES state)
{
	textureDesc = {};
	mByteSize = getSize(format);
	mFormat = format;
	mUsage = usage;
	mCubeMap = isCubeMap;
	D3D12_CLEAR_VALUE* tempclear = nullptr;

	if ((usage&D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)>0)
	{
		mClearVal.Format = format;
		mClearVal.Color[0] = clear.Color[0];
		mClearVal.Color[1] = clear.Color[1];
		mClearVal.Color[2] = clear.Color[2];
		mClearVal.Color[3] = clear.Color[3];
		tempclear = &mClearVal;
	}

	if ( (usage&D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)>0)
	{

		//if (mClearVal.Format == DXGI_FORMAT_R16_TYPELESS)
		//	mClearVal.Format = DXGI_FORMAT_D16_FLOAT;
		mClearVal.Format = format;
		if (mClearVal.Format == DXGI_FORMAT_R32_TYPELESS)
			mClearVal.Format = DXGI_FORMAT_D32_FLOAT;

		if (mClearVal.Format == DXGI_FORMAT_R16_TYPELESS)
			mClearVal.Format = DXGI_FORMAT_D16_UNORM;


		mClearVal.DepthStencil = clear.DepthStencil;
		tempclear = &mClearVal;
	}



	switch (dem)
	{
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.DepthOrArraySize = (UINT16)arraySize;
		if (mCubeMap)
		{
			textureDesc.DepthOrArraySize *=6;
			//textureDesc.Width = height *6;
		}
		textureDesc.MipLevels = mipLevel;
		textureDesc.Format = format;
		textureDesc.Flags = usage;

		textureDesc.Dimension = dem;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		break;
	}
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		state,
		tempclear,
		IID_PPV_ARGS(&mResource)));
	mState = state;
	//GpuAddress = mResource->GetGPUVirtualAddress();  because only buffer can read with out descriptor, so texture can't use gpuvirtualadress. Intereseting stuff
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mResource, 0, textureDesc.DepthOrArraySize*textureDesc.MipLevels);

//	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	CD3DX12_RESOURCE_DESC textdesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	//textdesc.SampleDesc.Count = 

	// Create the GPU upload buffer.
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mUploadBuffer)));



	//GpuAddress = mResource->GetGPUVirtualAddress();

}
void Texture::addSahderResorceView(DescriptorHeap& heap)
{	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
	srvDesc.Format = textureDesc.Format;
	if (srvDesc.Format == DXGI_FORMAT_R32_TYPELESS)
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	if (srvDesc.Format == DXGI_FORMAT_R16_TYPELESS)
		srvDesc.Format = DXGI_FORMAT_R16_UNORM;

	// need to decide the deimesnion depends on the real dimesion of texture
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	if (mCubeMap)
	{

		if (textureDesc.DepthOrArraySize > 6)  // cube texture array
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			srvDesc.TextureCubeArray.First2DArrayFace = 0;
			srvDesc.TextureCubeArray.NumCubes = textureDesc.DepthOrArraySize/6;
			srvDesc.TextureCubeArray.MipLevels = textureDesc.MipLevels;
			srvDesc.TextureCubeArray.MostDetailedMip = 0;
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = textureDesc.MipLevels;
			srvDesc.TextureCube.MostDetailedMip = 0;
		}
		//srvDesc.TextureCube.ResourceMinLODClamp = 0;
	}


	mSRV = heap.addResource(SRV, mResource, &srvDesc, nullptr);
}

void Texture::addDepgthStencilView(DescriptorHeap& heap, UINT level)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};

	
	depthStencilDesc.Format = textureDesc.Format;
	if (depthStencilDesc.Format == DXGI_FORMAT_R32_TYPELESS)
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	if (depthStencilDesc.Format == DXGI_FORMAT_R16_TYPELESS)
		depthStencilDesc.Format = DXGI_FORMAT_D16_UNORM;
	
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
//	depthStencilDesc.Texture2D.MipSlice = level;
	if (mCubeMap || textureDesc.DepthOrArraySize > 1)
	{
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	//	depthStencilDesc.Texture2DArray.MipSlice = level;
		depthStencilDesc.Texture2DArray.FirstArraySlice = 0;
		depthStencilDesc.Texture2DArray.ArraySize = textureDesc.DepthOrArraySize;
	//	depthStencilDesc.
	}
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	mDSV.push_back (heap.addResource(DSV, mResource, &depthStencilDesc, nullptr));
}
void Texture::addRenderTargetView(DescriptorHeap& heap)
{
//	D3D12_RENDER_TARGET_VIEW_DESC renDesc;
	mRTV.push_back( heap.addResource(RTV, mResource, nullptr));
}

void Texture::addUnorderedAccessView(DescriptorHeap& heap)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = mFormat;
	if (uavDesc.Format == DXGI_FORMAT_R32_TYPELESS)
		uavDesc.Format = DXGI_FORMAT_D32_FLOAT;
	if (uavDesc.Format == DXGI_FORMAT_R16_TYPELESS)
		uavDesc.Format = DXGI_FORMAT_R16_UNORM;

	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;
	mUAV.push_back (heap.addResource(UAV, mResource, &uavDesc, nullptr));
}

void Texture::CreateTexture(Render& render, DescriptorHeap& srvuavheap, DXGI_FORMAT format, UINT width, UINT height, UINT arraySize, UINT mipLevel,
	TEXURE_SRV_TYPE srvtype, TEXTURE_USAGE usage, D3D12_RESOURCE_STATES state, TEXTURE_ALL_MIPS_USE mipuse,
	ClearValue& clear)
{
	textureDesc = {};
	mByteSize = getSize(format);
	mFormat = format;
//	mUsage = usage;
	mUsage = D3D12_RESOURCE_FLAG_NONE;
//	mCubeMap = isCubeMap;

	mTextUsage = usage;
	mMipsUse = mipuse;
	mSRVType = srvtype;

	D3D12_CLEAR_VALUE* tempclear = nullptr;

	if ((usage&TEXTURE_USAGE_RTV)>0)
	{
		mClearVal.Format = format;
		if (clear.Type == ClearValue::CLEAR_TYPE_DEFAUTL)
		{
			mClearVal.Color[0] = 0.0f;
			mClearVal.Color[1] = 0.0f;
			mClearVal.Color[2] = 0.0f;
			mClearVal.Color[3] = 0.0f;
		}
		else
		{
			mClearVal.Color[0] = clear.Color[0];
			mClearVal.Color[1] = clear.Color[1];
			mClearVal.Color[2] = clear.Color[2];
			mClearVal.Color[3] = clear.Color[3];
		}
		tempclear = &mClearVal;
		mUsage |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}

	if ((usage&TEXTURE_USAGE_DSV)>0)
	{
		mClearVal.Format = format;
		if (mClearVal.Format == DXGI_FORMAT_R32_TYPELESS)
			mClearVal.Format = DXGI_FORMAT_D32_FLOAT;
		if (mClearVal.Format == DXGI_FORMAT_R16_TYPELESS)
			mClearVal.Format = DXGI_FORMAT_D16_UNORM;
		if (clear.Type == ClearValue::CLEAR_TYPE_DEFAUTL)
		{
			mClearVal.DepthStencil.Depth = 1.0;
			mClearVal.DepthStencil.Stencil = 0;
		}
		else
			mClearVal.DepthStencil = clear.DepthStencil;
		tempclear = &mClearVal;
		mUsage |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if ((usage&TEXTURE_USAGE_UAV) > 0)
	{
		mUsage |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.DepthOrArraySize = (UINT16)arraySize;
	if ((mSRVType&TEXURE_SRV_TYPE_CUBE)>0) // cube array
	{
		textureDesc.DepthOrArraySize *= 6;

	}
	textureDesc.MipLevels = mipLevel;
	textureDesc.Format = format;
	textureDesc.Flags = mUsage;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	if ((mSRVType&TEXURE_SRV_TYPE_3D)>0)
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;




	ThrowIfFailed(render.mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		state,
		tempclear,
		IID_PPV_ARGS(&mResource)));
	mState = state;
	//GpuAddress = mResource->GetGPUVirtualAddress();  because only buffer can read with out descriptor, so texture can't use gpuvirtualadress. Intereseting stuff
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mResource, 0, textureDesc.DepthOrArraySize*textureDesc.MipLevels);

	//	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	CD3DX12_RESOURCE_DESC textdesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);


	// Create the GPU upload buffer.
	ThrowIfFailed(render.mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mUploadBuffer)));

	createSRV(srvuavheap);
	if ((usage&TEXTURE_USAGE_DSV))
		createDSVs(render.mDSVDescriptorHeap);
	if ((usage&TEXTURE_USAGE_RTV))
		createRTVs(render.mRTVDescriptorHeap);
	if ((usage&TEXTURE_USAGE_UAV))
		createUAVs(srvuavheap);

}
void Texture::createSRV(DescriptorHeap& heap)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Format = textureDesc.Format; 
	if (srvDesc.Format == DXGI_FORMAT_R32_TYPELESS)
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	if (srvDesc.Format == DXGI_FORMAT_R16_TYPELESS)
		srvDesc.Format = DXGI_FORMAT_R16_UNORM;



	// need to decide the deimesnion depends on the real dimesion of texture
	if (mSRVType==TEXURE_SRV_TYPE_2D) // 2d could be 2d or 2d array
	{
		if (textureDesc.DepthOrArraySize == 1)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = textureDesc.DepthOrArraySize;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.MipLevels = textureDesc.MipLevels;
		}

	}

	if (mSRVType==TEXURE_SRV_TYPE_CUBE)
	{

		if (textureDesc.DepthOrArraySize > 6)  // cube texture array
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			srvDesc.TextureCubeArray.First2DArrayFace = 0;
			srvDesc.TextureCubeArray.NumCubes = textureDesc.DepthOrArraySize / 6;
			srvDesc.TextureCubeArray.MostDetailedMip = 0;
			srvDesc.TextureCubeArray.MipLevels = textureDesc.MipLevels;
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = textureDesc.MipLevels;
		}
	}

	if (mSRVType==TEXURE_SRV_TYPE_3D)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = textureDesc.MipLevels;
		srvDesc.Texture3D.MostDetailedMip = 0;
	}

	mSRV = heap.addResource(SRV, mResource, &srvDesc, nullptr);
}
void Texture::createRTVs(DescriptorHeap& heap)
{
	D3D12_RENDER_TARGET_VIEW_DESC renDesc = {};
	renDesc.Format = textureDesc.Format;

	if (renDesc.Format == DXGI_FORMAT_R32_TYPELESS)
		renDesc.Format = DXGI_FORMAT_R32_FLOAT;
	if (renDesc.Format == DXGI_FORMAT_R16_TYPELESS)
		renDesc.Format = DXGI_FORMAT_R16_UNORM;

	if (mSRVType== TEXURE_SRV_TYPE_3D)
	{
		renDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
		if((mMipsUse&TEXTURE_ALL_MIPS_USE_RTV) == 0)
			mRTV.push_back(heap.addResource(RTV, mResource, &renDesc, nullptr)); // only need level 0 case
		else
		{
			for (int j = 0; j < textureDesc.MipLevels; ++j)
			{
				renDesc.Texture3D.MipSlice = j;
				mRTV.push_back(heap.addResource(RTV, mResource, &renDesc, nullptr));
			}
		}
	}
	else if (textureDesc.DepthOrArraySize > 1)
	{
		renDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		renDesc.Texture2DArray.PlaneSlice = 0;
		renDesc.Texture2DArray.ArraySize = textureDesc.DepthOrArraySize;
		renDesc.Texture2DArray.FirstArraySlice = 0;
		if ((mMipsUse&TEXTURE_ALL_MIPS_USE_RTV) == 0)
			mRTV.push_back(heap.addResource(RTV, mResource, &renDesc, nullptr)); // only need level 0 case
		else
		{
			for (int j = 0; j < textureDesc.MipLevels; ++j)
			{
				renDesc.Texture2DArray.MipSlice = j;
				mRTV.push_back(heap.addResource(RTV, mResource, &renDesc, nullptr));
			}
		}
	}
	else
	{
		renDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		renDesc.Texture2D.PlaneSlice = 0;
		if ((mMipsUse&TEXTURE_ALL_MIPS_USE_RTV)==0)
			mRTV.push_back(heap.addResource(RTV, mResource, &renDesc, nullptr)); // only need level 0 case
		else
		{
			for (int j = 0; j < textureDesc.MipLevels; ++j)
			{
				renDesc.Texture2D.MipSlice = j;
				mRTV.push_back(heap.addResource(RTV, mResource, &renDesc, nullptr));
			}
		}
	}
}

void Texture::createDSVs(DescriptorHeap& heap)
{

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = textureDesc.Format;
	if (depthStencilDesc.Format == DXGI_FORMAT_R32_TYPELESS)
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	if (depthStencilDesc.Format == DXGI_FORMAT_R16_TYPELESS)
		depthStencilDesc.Format = DXGI_FORMAT_D16_UNORM;

	if (textureDesc.DepthOrArraySize > 1)
	{
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		depthStencilDesc.Texture2DArray.ArraySize = textureDesc.DepthOrArraySize;
		depthStencilDesc.Texture2DArray.FirstArraySlice = 0;
		if ((mMipsUse&TEXTURE_ALL_MIPS_USE_DSV) == 0)
			mDSV.push_back(heap.addResource(DSV, mResource, &depthStencilDesc, nullptr)); // only need level 0 case
		else
		{
			for (int j = 0; j < textureDesc.MipLevels; ++j)
			{
				depthStencilDesc.Texture2DArray.MipSlice = j;
				mDSV.push_back(heap.addResource(DSV, mResource, &depthStencilDesc, nullptr));
			}
		}
	}
	else
	{
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		if ((mMipsUse&TEXTURE_ALL_MIPS_USE_DSV)==0)
			mDSV.push_back(heap.addResource(DSV, mResource, &depthStencilDesc, nullptr)); // only need level 0 case
		else
		{
			for (int j = 0; j < textureDesc.MipLevels; ++j)
			{
				depthStencilDesc.Texture2D.MipSlice = j;
				mDSV.push_back(heap.addResource(DSV, mResource, &depthStencilDesc, nullptr));
			}
		}
	}

}
void Texture::createUAVs(DescriptorHeap& heap)
{
	cout << "UAV TEXTURE still Undone " << endl;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = textureDesc.Format;
	if (uavDesc.Format == DXGI_FORMAT_R32_TYPELESS)
		uavDesc.Format = DXGI_FORMAT_D32_FLOAT;
	if (uavDesc.Format == DXGI_FORMAT_R16_TYPELESS)
		uavDesc.Format = DXGI_FORMAT_R16_UNORM;

//	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	if (mSRVType == TEXURE_SRV_TYPE_3D)
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		if ((mMipsUse&TEXTURE_ALL_MIPS_USE_UAV)==0)
			mUAV.push_back(heap.addResource(UAV, mResource, &uavDesc, nullptr)); // only need level 0 case
		else
		{
			for (int j = 0; j < textureDesc.MipLevels; ++j)
			{
				uavDesc.Texture3D.MipSlice = j;
				mUAV.push_back(heap.addResource(UAV, mResource, &uavDesc, nullptr));
			}
		}
	}
	else if (textureDesc.DepthOrArraySize > 1)
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.PlaneSlice = 0;
		uavDesc.Texture2DArray.ArraySize = textureDesc.DepthOrArraySize;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		if ((mMipsUse&TEXTURE_ALL_MIPS_USE_UAV)==0)
			mUAV.push_back(heap.addResource(UAV, mResource, &uavDesc, nullptr)); // only need level 0 case
		else
		{
			for (int j = 0; j < textureDesc.MipLevels; ++j)
			{
				uavDesc.Texture2DArray.MipSlice = j;
				mUAV.push_back(heap.addResource(UAV, mResource, &uavDesc, nullptr));
			}
		}

	}
	else
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.PlaneSlice = 0;
		if ((mMipsUse&TEXTURE_ALL_MIPS_USE_UAV)==0)
			mUAV.push_back(heap.addResource(UAV, mResource, &uavDesc, nullptr)); // only need level 0 case
		else
		{
			for (int j = 0; j < textureDesc.MipLevels; ++j)
			{
				uavDesc.Texture2D.MipSlice = j;
				mUAV.push_back(heap.addResource(UAV, mResource, &uavDesc, nullptr));
			}
		}
	}


}