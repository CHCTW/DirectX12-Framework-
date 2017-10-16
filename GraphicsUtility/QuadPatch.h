#pragma once

#include "glm\vec4.hpp"
#include "vector"
enum YGenerateMode
{
	ZERO,
	PerlinGenerate
};
enum IndexGenerateMode
{
	Triangle,
	Quad,
	SmoothBezier
};
class QuadPatch
{
public:
	void generatePatch(float width, float height, unsigned int widthres, unsigned int heightres, IndexGenerateMode indexmode = Quad,YGenerateMode mode = ZERO,unsigned int seed = 0,float xfreqency = 1, float yfreqency = 1,unsigned int scale = 1);
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