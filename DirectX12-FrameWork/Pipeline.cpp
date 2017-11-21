#include "Pipeline.h"
#include "stdafx.h"
#include "StructureHeaders.h"
class Shader;

Pipeline::Pipeline():mPipeline(NULL)
{

}

bool Pipeline::createGraphicsPipeline(ID3D12Device* device, RootSignature& rootsig, ShaderSet & shaderset, RenderTargetFormat &format, DepthStencilState& depthstenstate, BlendState &blendstate, RasterizerState &raster, VertexInputLayOutType layout, D3D12_PRIMITIVE_TOPOLOGY_TYPE topype)
{
	mType = Graphics;
	//psoDesc = {};

	std::vector<D3D12_INPUT_ELEMENT_DESC> elemts;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	if (shaderset.shaders[VS].mShader)
	{




		psoDesc.VS = CD3DX12_SHADER_BYTECODE(shaderset.shaders[VS].mShader);
			elemts = shaderset.shaders[VS].getInputElements(layout);
			psoDesc.InputLayout = { elemts.data(),(unsigned int)elemts.size() };
	}
	if (shaderset.shaders[PS].mShader)
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(shaderset.shaders[PS].mShader);
	if (shaderset.shaders[GS].mShader)
		psoDesc.GS = CD3DX12_SHADER_BYTECODE(shaderset.shaders[GS].mShader);
	if (shaderset.shaders[DS].mShader)
		psoDesc.DS = CD3DX12_SHADER_BYTECODE(shaderset.shaders[DS].mShader);
	if (shaderset.shaders[HS].mShader)
		psoDesc.HS = CD3DX12_SHADER_BYTECODE(shaderset.shaders[HS].mShader);
	psoDesc.pRootSignature = rootsig.mRootSignature;
	psoDesc.RasterizerState = raster.mRasterizerState;
	psoDesc.BlendState = blendstate.mBlendState;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = topype;
	psoDesc.NumRenderTargets = format.mNumofRenderTargets;
	
	for(int i = 0 ; i < format.mNumofRenderTargets ; ++i)
		psoDesc.RTVFormats[i] = format.mRenderTargetFormat[i];



	psoDesc.DepthStencilState = depthstenstate.mDepthStencilState;
	psoDesc.DSVFormat = format.mDepthStencilFormat;
	if (psoDesc.DSVFormat == DXGI_FORMAT_R32_TYPELESS)
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	if (psoDesc.DSVFormat == DXGI_FORMAT_R16_TYPELESS)
		psoDesc.DSVFormat = DXGI_FORMAT_D16_UNORM;

//	psoDesc.DepthStencilState.DepthEnable = FALSE;
//	psoDesc.DepthStencilState.StencilEnable = FALSE;


	psoDesc.SampleDesc.Count = 1;




	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipeline));
	return true;
}

bool Pipeline::createComputePipeline(ID3D12Device* device, RootSignature& rootsig, ShaderSet & shaderset)
{
	mType = Compute;
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

	std::vector<D3D12_INPUT_ELEMENT_DESC> elemts;
	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	if (shaderset.shaders[CS].mShader)
	{
		psoDesc.CS = CD3DX12_SHADER_BYTECODE(shaderset.shaders[CS].mShader);

	}
	psoDesc.pRootSignature = rootsig.mRootSignature;
	ThrowIfFailed(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mPipeline)));
	return true;
}



void Pipeline::release()
{
	SAFE_RELEASE(mPipeline);
}