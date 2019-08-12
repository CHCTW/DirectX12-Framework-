#pragma once
#include "d3dx12.h"
#include "stdafx.h"
#include <d3d12shader.h>
#include <string>
#include <vector>
#include <utility>
#include <ProtoBuf/pipeline.pb.h>
class Shader
{
public:

	bool load(std::string const&& filepath, std::string  const&& entryPoint, ShaderType type);
	bool load(const std::string& filepath, const std::string& entryPoint, ShaderType type)
	{
		return load(std::move(filepath), std::move(entryPoint), type);
	}
	Shader();
	std::vector<D3D12_INPUT_ELEMENT_DESC> getInputElements(VertexInputLayOutType layout);
	ID3DBlob* mShader;
	ShaderType mType;
	std::string mFilePath;
	std::string mEntryPoint;
	ID3D12ShaderReflection* mReflector;
	D3D12_SHADER_DESC mShaderDesc;
	template<typename T>
	bool loadFromProto(const T& shader)
	{
		return load_from_proto_(std::move(shader));
	}
private:
	//bool load_(string const& filepath, string const& entryPoint, ShaderType type);
	bool load_from_proto_(const Config::Pipeline::Shader&& shader);

};