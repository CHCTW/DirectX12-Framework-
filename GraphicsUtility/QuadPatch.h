#pragma once

#include "glm\vec4.hpp"
#include "vector"
class QuadPatch
{
public:
	void generatePatch(float width, float height, unsigned int widthres, unsigned int heightres);
	float mWidth;
	float mHeight;
	unsigned int mHeightRes;
	unsigned int mWidthRes;
	std::vector<float> mPosition;
	std::vector<float> mNormal;
	std::vector<float> mUV;
	std::vector<unsigned int> mIndex;
private:
};