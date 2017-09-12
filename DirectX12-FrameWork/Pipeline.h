#pragma once
#include <vector>
#include "d3dx12.h"
#include <unordered_map>
#include "Shader.h"
#include "stdafx.h"
enum PipelineType
{
	Graphics,
	Compute
};
struct ShaderSet {
	std::unordered_map<ShaderType, Shader>  shaders;
};
class DepthStencilState;
class BlendState;
class RasterizerState;
class Shader;
class RootSignature;
class Pipeline
{
public:
	Pipeline();
	bool createGraphicsPipeline(ID3D12Device* device, RootSignature& rootsig, ShaderSet & shaders, RenderTargetFormat &format, 
		DepthStencilState& depthstenstate,BlendState &blendstate,RasterizerState &raster, VertexInputLayOutType layout = VERTEX_LAYOUT_TYPE_NONE_SPLIT, D3D12_PRIMITIVE_TOPOLOGY_TYPE topype = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	bool createComputePipeline(ID3D12Device* device, RootSignature& rootsig, ShaderSet & shaders);

	void release();
	ID3D12PipelineState* mPipeline;
	PipelineType mType;
	
};