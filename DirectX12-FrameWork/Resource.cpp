#include "Resource.h"
void Resource::release()
{

	mRTV.clear();
	mDSV.clear();
	mUAV.clear();
	SAFE_RELEASE(mResource);
	SAFE_RELEASE(mUploadBuffer);
}