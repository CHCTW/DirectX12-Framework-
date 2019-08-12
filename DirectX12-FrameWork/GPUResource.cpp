#include "GPUResource.h"
void GPUResource::release()
{

	mRTV.clear();
	mDSV.clear();
	mUAV.clear();
	SAFE_RELEASE(mResource);
	SAFE_RELEASE(mUploadBuffer);
}