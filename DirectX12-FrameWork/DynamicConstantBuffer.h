#pragma once
#include "DynamicUploadBuffer.h"
class DynamicConstantBuffer
{
public:
	void initialize(Render& render, UINT64 startsize = 1000);
	void release();
	void setCurrentFrameNumber(UINT64 frame);
	void freeAllocateUntilFrame(UINT64 frame);
	// can allocate constant buffer , but don't have ability to change the content after allocate
	VolatileConstantBuffer const allocateVolatileConstantBuffer(void const * data, UINT64 datasize);
private:
	AllocateFormat allocateforCurrentFrame(UINT64 reqsize);
	ID3D12Device * mDevice;
	deque<RingBuffer> ringbuffers; // store for ring buffers, the last one is the largest ring buffer
	std::mutex cslock; //  for multi-threading safe, since recording command list can run on differnt thread
	UINT64 currentframe;
	UINT64 prevacccallocsize;
	UINT64 accallocsize;
};