#pragma once
#include "d3dx12.h"
#include "stdafx.h"
#include <d3d12shader.h>
#include <string>
#include <vector>


class Shader
{
public:
	bool load(char* filepath, char* entryPoint,ShaderType type);
	Shader();
	std::vector<D3D12_INPUT_ELEMENT_DESC> getInputElements(VertexInputLayOutType layout);
	ID3DBlob* mShader;
	ShaderType mType;
	ID3D12ShaderReflection* mReflector;
	D3D12_SHADER_DESC mShaderDesc;

};