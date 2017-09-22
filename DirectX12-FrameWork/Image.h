#pragma once
#include <vector>
#include "stdafx.h"
class Image
{
public:
	Image():mData(nullptr), floatType(false)
	{

	}
	~Image()
	{
		if (mData)
			delete [] mData;
	}
	bool load(const char* filename, unsigned int miplevel = 1,unsigned int channel = 4);
	unsigned char* mData;

	int mWidth;
	int mHeight;
	int mChannel;
	int mMips;
	bool floatType;
	int elemntSize;
private:
	bool generateMipmaps(unsigned int miplevel);
	template
	<typename T>
	void downSample(T* originaldata,T* downsampledata,unsigned int width,unsigned height)
	{
		unsigned int xoffset = mChannel;
		if (width == 1)
			xoffset = 0;
		unsigned int yoffset = mChannel*width;
		if (height == 1)
			yoffset = 0;
		for (unsigned int i = 0; i < height / 2; ++i)
		{
			for (unsigned int j = 0; j < width / 2; ++j)
			{
				for (unsigned p = 0; p < mChannel; ++p)
				{
					downsampledata[p] = (originaldata[p] + originaldata[p + xoffset] + originaldata[p + yoffset] + originaldata[p + yoffset + xoffset]) / 4;
				}
				originaldata += 2 * xoffset; // move 2 pixel;
				downsampledata += xoffset;
			}
			originaldata += yoffset; // move down one line;
		}

	}
	
};