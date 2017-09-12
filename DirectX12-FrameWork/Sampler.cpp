#include "Sampler.h"
#include "StructureHeaders.h"

bool Sampler::createSampler(DescriptorHeap & heap)
{
	mSampler = heap.addSampler(mSamplerDesc);
	return true;
}
