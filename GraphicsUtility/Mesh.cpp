#include "stdafx.h"
#include "Mesh.h"
#include "Render.h"
#include <limits>
using namespace std;
#undef  min()
#undef  max()
bool Mesh::loadMesh(aiMesh* assmesh, Render& render, CommandAllocator& cmdalloc, CommandList &cmdlist)
{
	Fence tempfence;
	tempfence.initialize(render);
	if (!assmesh)
		return false;
	if (assmesh->HasPositions())
	{
		mPositionBuffer.createVertexBuffer(render.mDevice, assmesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
		mMax.x = std::numeric_limits<float>::min();
		mMax.y = std::numeric_limits<float>::min();
		mMax.z = std::numeric_limits<float>::min();
		mMin.x = std::numeric_limits<float>::max();
		mMin.y = std::numeric_limits<float>::max();
		mMin.z = std::numeric_limits<float>::max();
		for (int i = 0; i < assmesh->mNumVertices; i++) // find min, max
		{
			mMax.x = std::max(assmesh->mVertices[i].x, mMax.x);
			mMax.y = std::max(assmesh->mVertices[i].y, mMax.y);
			mMax.z = std::max(assmesh->mVertices[i].z, mMax.z);

			mMin.x = std::min(assmesh->mVertices[i].x, mMin.x);
			mMin.y = std::min(assmesh->mVertices[i].y, mMin.y);
			mMin.z = std::min(assmesh->mVertices[i].z, mMin.z);
		}

	}
	//if(assmesh->HasNormals())
	mNormalBuffer.createVertexBuffer(render.mDevice, assmesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
//	if(assmesh->HasTextureCoords(0))
	mUVBuffer.createVertexBuffer(render.mDevice, assmesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	//if(assmesh->HasTangentsAndBitangents())
	mTangentBuffer.createVertexBuffer(render.mDevice, assmesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	mBiTangentBuffer.createVertexBuffer(render.mDevice, assmesh->mNumVertices * 3 * sizeof(float), 3 * sizeof(float));
	mIndexBuffer.createIndexBuffer(render.mDevice, sizeof(unsigned int) * 3 * assmesh->mNumFaces);
	std::vector<unsigned int> indexdata;
	indexdata.resize(assmesh->mNumFaces * 3);
	for (int i = 0; i < assmesh->mNumFaces; i++)
	{
		indexdata[i * 3] = assmesh->mFaces[i].mIndices[0];
		indexdata[i * 3 + 1] = assmesh->mFaces[i].mIndices[1];
		indexdata[i * 3 + 2] = assmesh->mFaces[i].mIndices[2];
	}
	indexCount = indexdata.size();
	startIndex = 0;

	cmdalloc.reset();
	cmdlist.reset();

	// not a good way to write it, cause I don't have batch barrier yet. since I only assume this only happened in loading time, it probally not too bad now
	if (assmesh->HasPositions())
	{
		cmdlist.resourceTransition(mPositionBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
		cmdlist.updateBufferData(mPositionBuffer, assmesh->mVertices, assmesh->mNumVertices * 3 * sizeof(float));
		cmdlist.resourceTransition(mPositionBuffer,D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,true);

	

	}
	if (assmesh->HasNormals())
	{
		cmdlist.resourceTransition(mNormalBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
		cmdlist.updateBufferData(mNormalBuffer, assmesh->mNormals, assmesh->mNumVertices * 3 * sizeof(float));
		cmdlist.resourceTransition(mNormalBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,true);
	}
	if (assmesh->HasTextureCoords(0))
	{
		cmdlist.resourceTransition(mUVBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
		cmdlist.updateBufferData(mUVBuffer, assmesh->mTextureCoords[0], assmesh->mNumVertices * 3 * sizeof(float));
		cmdlist.resourceTransition(mUVBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,true);
	}
	if (assmesh->HasTangentsAndBitangents())
	{
		cmdlist.resourceTransition(mTangentBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
		cmdlist.updateBufferData(mTangentBuffer, assmesh->mTangents, assmesh->mNumVertices * 3 * sizeof(float));
		cmdlist.resourceTransition(mTangentBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,true);

		cmdlist.resourceTransition(mBiTangentBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
		cmdlist.updateBufferData(mBiTangentBuffer, assmesh->mBitangents, assmesh->mNumVertices * 3 * sizeof(float));
		cmdlist.resourceTransition(mBiTangentBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,true);

	}
	cmdlist.resourceTransition(mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,true);
	cmdlist.updateBufferData(mIndexBuffer, indexdata.data(), assmesh->mNumFaces * 3 * sizeof(unsigned int));
	cmdlist.resourceTransition(mIndexBuffer,D3D12_RESOURCE_STATE_INDEX_BUFFER,true);


	cmdlist.close();
	render.executeCommands(&cmdlist);
	//render.waitCommandsDone();
	render.insertSignalFence(tempfence);
	render.waitFence(tempfence);
	tempfence.release();
//	if(assmesh->hahasf)

	return true;
}

IndirectMeshData Mesh::getIndirectData()
{
	IndirectMeshData data;
	if (mPositionBuffer.mResource)
		data.mPosition = mPositionBuffer.mVertexBuffer;
	if (mNormalBuffer.mResource)
		data.mNormal = mNormalBuffer.mVertexBuffer;
	if (mUVBuffer.mResource)
		data.mUV = mUVBuffer.mVertexBuffer;
	if (mTangentBuffer.mResource)
		data.mTangent = mTangentBuffer.mVertexBuffer;

	if (mBiTangentBuffer.mResource)
		data.mBiTangent = mBiTangentBuffer.mVertexBuffer;

	data.mIndex = mIndexBuffer.mIndexBuffer;
	data.startIndex = startIndex;
	data.indexCount = indexCount;
	data.mMax = mMax;
	data.mMin = mMin;
	return data;
	
}

void Mesh::release()
{
	mPositionBuffer.release();
	mUVBuffer.release();
	mNormalBuffer.release();
	mTangentBuffer.release();
	mBiTangentBuffer.release();
	mIndexBuffer.release();
}