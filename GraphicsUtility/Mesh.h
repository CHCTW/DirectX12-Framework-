#pragma once
#include "stdafx.h"
#include "glm\vec4.hpp"
#include <vector>
#include <assimp\ai_assert.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\mesh.h>
#include <string>
#include "Buffer.h"
class Render;
class CommandAllocator;
class CommandList;
// structer for indirect draw argument
struct IndirectMeshData
{
	VertexBufferView mPosition;
	VertexBufferView mNormal;
	VertexBufferView mUV;
	VertexBufferView mTangent;
	VertexBufferView mBiTangent;
	IndexBufferView mIndex;
	unsigned int indexCount;
	unsigned int startIndex;
	glm::vec3 mMin; // bounding box
	glm::vec3 mMax;
};
class Mesh 
{
public:
	Mesh()
	{

	}
	~Mesh()
	{
	}
	bool loadMesh(aiMesh* assmesh,Render& render,CommandAllocator& cmdalloc,CommandList &cmdlist);
	void release();
	IndirectMeshData getIndirectData();
	//bool bindIndirectData(IndirectMesh* indirectdata);
	//bool updateIndirectData(IndirectMesh* indirectdata);
	Buffer mPositionBuffer;
	Buffer mUVBuffer;
	Buffer mNormalBuffer;
	Buffer mTangentBuffer;
	Buffer mBiTangentBuffer;
	Buffer mIndexBuffer;
	unsigned int indexCount;
	unsigned int startIndex;
	glm::vec3 mMin; // bouning box
	glm::vec3 mMax;
	//IndirectMesh* IndirectData;
};