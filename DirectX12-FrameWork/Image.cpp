#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>
#include <string>
bool Image::load(const char* filename, unsigned int miplevel, unsigned int channel)
{

	if (mData)
		delete[] mData;
	int bpp;
	std::string name(filename);
	int size = name.size();
	if (name[size - 3] == 'h' && name[size - 2] == 'd' && name[size - 1] == 'r') // hdr file
	{
		float* tempfloat = stbi_loadf(filename, &mWidth, &mHeight, &bpp, channel);
		floatType = true;
		elemntSize = channel * 4;

		if (!tempfloat)
			return false;
		mData = new unsigned char[mWidth*mHeight*channel*4];
		memcpy(mData, tempfloat, mWidth*mHeight * channel*4);
		stbi_image_free(tempfloat);

	}
	else
	{
		unsigned char* temp = stbi_load(filename, &mWidth, &mHeight, &bpp, channel);
		if (!temp)
			return false;
		elemntSize = channel;
		mData = new unsigned char[mWidth*mHeight*channel];
		memcpy(mData, temp, mWidth*mHeight * channel);
		stbi_image_free(temp);
	}

	mChannel = channel;
	mMips = miplevel;
	if (miplevel != 1)
		generateMipmaps(mMips);
	return true;
}
bool Image::generateMipmaps(unsigned int miplevel)
{
	if (mWidth%2 | mHeight%2) // check for image need to be power of 2
		return false;
	unsigned w, h,total;
	w = mWidth;
	h = mHeight;
	total = 0;
	unsigned int maxmips = 0;
	for (int i = 0; i < mMips; ++i) // count mip size
	{
		maxmips++;
		total += w*h;
		w = w / 2;
		h = h / 2;
		if (w % 2 | h % 2)
			break;

		if (w < 1 || h < 1)
			break;
	}  
	
	mMips = maxmips;

	unsigned char* newdata = new unsigned char[total*elemntSize]; // allcate for new data
	memcpy(newdata, mData, mWidth*mHeight*elemntSize); // copy to the fist part of new data
	unsigned char* original = newdata;
	unsigned char* downsample = newdata + mWidth*mHeight*elemntSize;
	w = mWidth;
	h = mHeight;

	for (unsigned int i = 1; i < mMips; ++i)
	{
		if(floatType)
			downSample((float*)original, (float*)downsample, w, h);
		else
			downSample(original, downsample, w, h);
		w /= 2;
		h /= 2;
		original = downsample;
		downsample += h*w*elemntSize;
	}
	delete[] mData;
	mData = newdata;
	return true;


}

