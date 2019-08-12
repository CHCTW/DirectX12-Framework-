#pragma once
#include "stdafx.h"
#include <vector>
#include <deque>
#include <mutex>
using namespace std;
class Render;
class CommandList;

struct AllocateFormat
{
	ID3D12Resource* gpubuffer;
	void* cpubuffer;
	UINT64 offset;
};

struct RingBuffer
{
	void* cpubuffer;
	ID3D12Resource* gpubuffer;
	// size+frame
	deque<pair<UINT64, UINT64>> records;
	UINT64 head;
	UINT64 tail;
	UINT64 size;
	UINT64 unusedsize;
	bool valid;
	void initialize(UINT64 s, ID3D12Device* device);
	void release();
	bool enoughcontspace(UINT64 s);
	// offset + size
	UINT64 allocte(UINT64 requestsize,UINT64 frame);
	void free(UINT64 frame);
};
class VolatileConstantBuffer 
{
public:
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;
	UINT64 mSize;
	UINT64 mFrame;
};
class DynamicUploadBuffer
{
public:
	void initialize(Render& render, UINT64 startsize = 1000);
	void release();
	AllocateFormat allocateforCurrentFrame(UINT64 reqsize);

	//should only call once per frame
	void setCurrentFrameNumber(UINT64 frame);
	void freeAllocateUntilFrame(UINT64 frame);
	
private:
	ID3D12Device* mDevice;
	deque<RingBuffer> ringbuffers; // store for ring buffers, the last one is the largest ring buffer
	std::mutex cslock; //  for multi-threading safe, since recording command list can run on differnt thread
	UINT64 currentframe;
	UINT64 prevacccallocsize;
	UINT64 accallocsize;
};