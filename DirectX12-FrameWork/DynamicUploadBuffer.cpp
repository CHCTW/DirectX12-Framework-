#include "DynamicUploadBuffer.h"
#include "stdafx.h"
#include "StructureHeaders.h"
void RingBuffer::initialize(UINT64 s, ID3D12Device* device)
{
	size = s;
	unusedsize = s;
//	cpubuffer = new char[size];
	records.clear();
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(s),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&gpubuffer));
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	gpubuffer->Map(0, &readRange, &cpubuffer);
	head = 0;
	tail = 0;
	valid = true;
}
void RingBuffer::release()
{
	CD3DX12_RANGE readRange(0, 0);
	cpubuffer = nullptr;
	gpubuffer->Unmap(0, &readRange);
	gpubuffer->Release();
	valid = false;
	head = 0;
	tail = 0;
	size = 0;

	records.clear();
}
bool RingBuffer::enoughcontspace(UINT64 s)
{
	if (s > unusedsize)
		return false;
	if (tail >= head)
	{
		if (tail + s > size) // reach the end of the buffer
		{
			return (head >= s); // from start to head, check have enough space
		}
		return true;
	}
	else
	{
		return ((head - tail) >= s);
	}
}
// won't check unused size is large enough
UINT64 RingBuffer::allocte(UINT64 requestsize, UINT64 frame)
{
	UINT64 offset = 0;
	// normal case, just need to check whether it touch the end
	if (tail >= head)
	{
		if (tail + requestsize > size) // reach the end of the buffer, need to split to two parts, only going to use the second part
		{
			unsigned int remain = size - tail; // still need to allocate it to prevent fragment
		/*	allocata.mapcount = 1;
			allocata.maplist[0].first = 0;
			allocata.maplist[0].second = requestsize;
		*/
			offset = 0; // allocate from start
			tail = requestsize;
			requestsize += remain; // add up 
			
		}
		else  // have enough space
		{
			offset = tail;
			tail = (tail + requestsize)%size;
		}

	}
	else // teail is before head, since not going to check large enough, just allocate an area
	{
		/*allocata.mapcount = 1;
		allocata.maplist[0].first = tail;
		allocata.maplist[0].second = requestsize;*/
		offset = tail;
		tail = (tail + requestsize) % size;
	}
	unusedsize -= requestsize;
//	cout << requestsize << endl;
	if (records.size()==0 || records.back().second != frame) // current allocate the first allocate in his frame
	{
		pair<UINT64, UINT64> allorec;
		allorec.first = requestsize;
		allorec.second = frame;
		records.push_back(allorec);
	}
	else // already have allocate in this frame
	{
		records.back().first += requestsize; // just add up alloc size
	}


	return offset;
}
void RingBuffer::free(UINT64 frame)
{
	UINT64 freesize = 0;
	while (records.size() != 0 && records.front().second <= frame) // free all alocate that is before current frame
	{
		freesize += records.front().first; // add up clean up size
		records.pop_front(); // clean the record
	}
	if (head >= tail)
	{
		if (head + freesize > size)
		{
	//		unsigned int remain = freesize-(size - head);
			head = (freesize - (size - head)) %size;
		}
		else
		{
			head = (head + freesize) % size;
		}
	}
	else
		head = (head + freesize) % size;
	unusedsize +=freesize;
}

void DynamicUploadBuffer::initialize(Render& render, UINT64 startsize)
{
	mDevice = render.mDevice;
	ringbuffers.push_back(RingBuffer());
	ringbuffers.back().initialize(startsize, mDevice);
	currentframe = 0;
	accallocsize = 0;
	prevacccallocsize = 0;
}
void DynamicUploadBuffer::release()
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
AllocateFormat DynamicUploadBuffer::allocateforCurrentFrame(UINT64 reqsize)
{
	AllocateFormat format;

	cslock.lock();// protect for multi threading cmd list
	accallocsize += reqsize;
	if (!ringbuffers.back().enoughcontspace(reqsize)) // don't have enough con't size
	{
		ringbuffers.push_back(RingBuffer());
		ringbuffers.back().initialize((prevacccallocsize+reqsize) * 3, mDevice);// assume swap chain count is three, if it's stable no new ring buffer will be allocate
	}
	format.offset = ringbuffers.back().allocte(reqsize, currentframe);
	cslock.unlock();

	format.gpubuffer = ringbuffers.back().gpubuffer;
	format.cpubuffer = ringbuffers.back().cpubuffer;
	return format;
}
void DynamicUploadBuffer::setCurrentFrameNumber(UINT64 frame)
{
	cslock.lock();
	prevacccallocsize = accallocsize;
	accallocsize = 0;
	currentframe = frame;
	cslock.unlock();
}
void DynamicUploadBuffer::freeAllocateUntilFrame(UINT64 frame)
{
	//free first
	cslock.lock(); // usually it won't need this lock, since this is not going to happenend on cmdlist thread
	for (auto i = ringbuffers.begin(); i != ringbuffers.end(); ++i)
	{
		i->free(frame);
	}
	//release unused ring buffer except the last one
	while (ringbuffers.size() > 1&& ringbuffers.front().unusedsize== ringbuffers.front().size) // delete unused and no allocate buffer
	{
		ringbuffers.front().release(); // free
	//	cout << "Free ring buffers" << endl;
		ringbuffers.pop_front(); //delete
	}
	cslock.unlock();
}