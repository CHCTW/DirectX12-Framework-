#include "DynamicConstantBuffer.h"
#include "stdafx.h"
#include "StructureHeaders.h"
void DynamicConstantBuffer::initialize(Render& render, UINT64 startsize)
{
	mDevice = render.mDevice;
	ringbuffers.push_back(RingBuffer());
	ringbuffers.back().initialize(startsize, mDevice);
	currentframe = 0;
	accallocsize = 0;
	prevacccallocsize = 0;
}
void DynamicConstantBuffer::release()
{
	mDevice = nullptr;
	while (ringbuffers.size() != 0)
	{
		ringbuffers.front().release();
		ringbuffers.pop_front();
	}
	prevacccallocsize = 0;
	currentframe = 0;
	accallocsize = 0;
}

AllocateFormat  DynamicConstantBuffer::allocateforCurrentFrame(UINT64 reqsize)
{
	AllocateFormat format;

	cslock.lock();// protect for multi threading cmd list
	accallocsize += reqsize;
	if (!ringbuffers.back().enoughcontspace(reqsize)) // don't have enough con't size
	{
		ringbuffers.push_back(RingBuffer());
		ringbuffers.back().initialize((prevacccallocsize + reqsize) * 3, mDevice);// assume swap chain count is three, if it's stable no new ring buffer will be allocate
	}
	format.offset = ringbuffers.back().allocte(reqsize, currentframe);
	cslock.unlock();

	format.gpubuffer = ringbuffers.back().gpubuffer;
	format.cpubuffer = ringbuffers.back().cpubuffer;
	return format;
}
VolatileConstantBuffer const DynamicConstantBuffer::allocateVolatileConstantBuffer(void const * data, UINT64 datasize)
{
	VolatileConstantBuffer vcb;
	AllocateFormat f = allocateforCurrentFrame((datasize + 255) & ~255); // cbv aligment

	vcb.GpuAddress = f.gpubuffer->GetGPUVirtualAddress() + f.offset;

	char * cpuaddress = (char *)f.cpubuffer + f.offset;
	memcpy(cpuaddress, data, datasize);// copy data to buffer
	vcb.mSize = datasize;
	vcb.mFrame = currentframe;
	return vcb;
}
void DynamicConstantBuffer::setCurrentFrameNumber(UINT64 frame)
{
	cslock.lock();
	prevacccallocsize = accallocsize;
	accallocsize = 0;
	currentframe = frame;
	cslock.unlock();
}
void DynamicConstantBuffer::freeAllocateUntilFrame(UINT64 frame)
{
	//free first
	cslock.lock(); // usually it won't need this lock, since this is not going to happenend on cmdlist thread
	for (auto i = ringbuffers.begin(); i != ringbuffers.end(); ++i)
	{
		i->free(frame);
	}
	//release unused ring buffer except the last one
	while (ringbuffers.size() > 1 && ringbuffers.front().unusedsize == ringbuffers.front().size) // delete unused and no allocate buffer
	{
		ringbuffers.front().release(); // free
									   //	cout << "Free ring buffers" << endl;
		ringbuffers.pop_front(); //delete
	}
	cslock.unlock();
}