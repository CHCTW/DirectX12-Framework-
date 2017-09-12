#include "Resource.h"
void Resource::release()
{
	SAFE_RELEASE(mResource);
	SAFE_RELEASE(mUploadBuffer);
}