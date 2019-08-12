#include "Shader.h"
#include "stdafx.h"
#include <d3dcompiler.h>
#include <cwchar>
#include <vector>
ShaderType convertFromProtoShaderType(Config::Pipeline::Shader::Type type)
{
	ShaderType ret(ShaderType::VS);
	switch (type)
	{
		case Config::Pipeline::Shader::Type::Pipeline_Shader_Type_VERTEX:
		ret = ShaderType::VS;
		break;
	case Config::Pipeline::Shader::Type::Pipeline_Shader_Type_PIXEL:
		ret = ShaderType::PS;
		break;
	case Config::Pipeline::Shader::Type::Pipeline_Shader_Type_GEOMETRY:
		ret = ShaderType::GS;
		break;
	case Config::Pipeline::Shader::Type::Pipeline_Shader_Type_COMPUTE:
		ret = ShaderType::CS;
		break;
	case Config::Pipeline::Shader::Type::Pipeline_Shader_Type_TESSELATION_DOMAIN:
		ret = ShaderType::DS;
		break;
	case Config::Pipeline::Shader::Type::Pipeline_Shader_Type_TESSELATION_HULL:
		ret = ShaderType::HS;
		break;
	default:
		break;
	}
	return ret;
}
Shader::Shader():mType(SHADERTYPE_COUNT), mShader(NULL)
{

}
bool Shader::load(std::string const&& filepath, std::string const&& entryPoint, ShaderType type)
{
	mFilePath = filepath;
	mEntryPoint = entryPoint;
	std::wstring strins;
	strins.resize(256);
	WCHAR wsz[256];
	ID3DBlob* error;
	swprintf(wsz,256, L"%S", filepath.c_str());
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION| D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES| D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#else
	UINT compileFlags = D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES| D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#endif
	HRESULT hr = D3DCompileFromFile(wsz, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), Targetchars[type], compileFlags, 0, &mShader, &error);
	if (error)
	{
		OutputDebugStringA(reinterpret_cast<const char*>(error->GetBufferPointer()));
		error->Release();
	}

	ThrowIfFailed(hr);


	
	ThrowIfFailed(D3DReflect(mShader->GetBufferPointer(), mShader->GetBufferSize(), IID_PPV_ARGS(&mReflector)));

	ThrowIfFailed(mReflector->GetDesc(&mShaderDesc));
	//D3D12_SIGNATURE_PARAMETER_DESC inputdesc;
	//for (int i = 0; i < mShaderDesc.InputParameters; i++)
	//{
	//	
	//	mReflector->GetInputParameterDesc(i,&inputdesc);
	//}
	mType = type;
	return true;
	
}
std::vector<D3D12_INPUT_ELEMENT_DESC> Shader::getInputElements(VertexInputLayOutType layout)
{


	std::vector<D3D12_INPUT_ELEMENT_DESC> inputs;
	D3D12_SIGNATURE_PARAMETER_DESC inputdesc;
	D3D12_INPUT_ELEMENT_DESC inputelemt;
	unsigned int byteoffset = 0;
	UINT slot = 0;
	string indstancename("SV_InstanceID");
	string indexname("SV_VertexID");
	string primitivename("SV_PrimitiveID");
	for (int i = 0; i < mShaderDesc.InputParameters; i++)
	{
		
		mReflector->GetInputParameterDesc(i,&inputdesc);
		inputelemt.SemanticName = inputdesc.SemanticName;
		string t(inputelemt.SemanticName);
		if (t == indstancename || t == indexname || t == primitivename)
			continue;
	//	if(inputelemt.SemanticName == L"Test String")
		inputelemt.SemanticIndex = inputdesc.SemanticIndex;
		inputelemt.InputSlot = slot;
		if (layout == VERTEX_LAYOUT_TYPE_SPLIT_ALL)  // all input is in differnt buffer, byteoffset is always zero and change slot
		{
			++slot;
			byteoffset = 0;
		}
		inputelemt.AlignedByteOffset = byteoffset;
		inputelemt.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		inputelemt.InstanceDataStepRate = 0;
		switch (inputdesc.Mask)
		{
		case 1 :
			if(inputdesc.ComponentType== D3D_REGISTER_COMPONENT_UINT32) inputelemt.Format = DXGI_FORMAT_R32_UINT;
			else if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputelemt.Format = DXGI_FORMAT_R32_SINT;
			else if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputelemt.Format = DXGI_FORMAT_R32_FLOAT;
			byteoffset += 4;
			inputs.push_back(inputelemt);
			break;
		case 3:
			if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputelemt.Format = DXGI_FORMAT_R32G32_UINT;
			else if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputelemt.Format = DXGI_FORMAT_R32G32_SINT;
			else if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputelemt.Format = DXGI_FORMAT_R32G32_FLOAT;
			byteoffset += 8;
			inputs.push_back(inputelemt);
			break;
		case 7:
			if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputelemt.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputelemt.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputelemt.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			byteoffset += 12;
			inputs.push_back(inputelemt);
			break;
		case 15:
			if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputelemt.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputelemt.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (inputdesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputelemt.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			byteoffset += 16;
			inputs.push_back(inputelemt);
			break;
		}
		
	}

	return inputs;
}
bool Shader::load_from_proto_(const Config::Pipeline::Shader&& shader)
{
	//return this->load_(shader.path(), shader.entry_point(), convertFromProtoShaderType(shader.type()));
	return true;
}
