#include "CommandSignature.h"
#include "RootSignature.h"
bool CommandSignature::initialize(ID3D12Device* device,RootSignature& rootsig)
{
	UINT byteStride = 0;
	D3D12_INDIRECT_ARGUMENT_TYPE type;
	unsigned int alighment = 4;
	for (int i = 0; i < mParameters.size(); ++i)
	{
		switch (mParameters[i].Type)
		{
		case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW:
			byteStride += sizeof(D3D12_DRAW_ARGUMENTS);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
			byteStride += sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH:
			byteStride += sizeof(D3D12_DISPATCH_ARGUMENTS);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW:
			byteStride += sizeof(D3D12_VERTEX_BUFFER_VIEW);
			if (byteStride & 8 - 1 != 0) // not 8 btye alighment, need to add padding to byte size
				byteStride = (byteStride + 7)&~8;
			alighment = 8;
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW:
			byteStride += sizeof(D3D12_INDEX_BUFFER_VIEW);
			if (byteStride & 8 - 1 != 0) // not 8 btye alighment, need to add padding to byte size
				byteStride = (byteStride + 7)&~8;
			alighment = 8;
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT:
			byteStride += mParameters[i].Constant.Num32BitValuesToSet * sizeof(unsigned int);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW: // use pure gpu address only
			byteStride += sizeof(GpuAddress);
			if (byteStride & 8-1 != 0) // not 8 btye alighment, need to add padding to byte size
				byteStride  = (byteStride + 7)&~8;
			alighment = 8;
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW:
			byteStride += sizeof(GpuAddress);
			if (byteStride & 8-1 != 0) // not 8 btye alighment, need to add padding to byte size
				byteStride = (byteStride + 7)&~8;
			alighment = 8;
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW:
			byteStride += sizeof(GpuAddress);
			if (byteStride & 8-1 != 0) // not 8 btye alighment, need to add padding to byte size
				byteStride = (byteStride + 7)&~8;
			alighment = 8;
			break;
		default:
			break;
		}

	}
	byteStride += paddingsize;
	if (alighment==8)
		if((byteStride & (8 - 1)) > 0) // not 8 btye alighment, need to add padding to byte size
		byteStride = (byteStride + 7)& ~7;
	cout << (byteStride & (8 - 1)) << endl;
	mCommandSignatureDes.ByteStride = byteStride;
	mCommandSignatureDes.pArgumentDescs = mParameters.data();
	mCommandSignatureDes.NumArgumentDescs = mParameters.size();
	mCommandSignatureDes.NodeMask = 0;

	ThrowIfFailed(device->CreateCommandSignature(&mCommandSignatureDes, rootsig.mRootSignature, IID_PPV_ARGS(&mCommandSignature)));
	return true;
}

void CommandSignature::release()
{
	SAFE_RELEASE(mCommandSignature);
}