#include "stdafx.h"
#include "QuadPatch.h"
#include <glm/vec4.hpp>
#include <iostream>
using namespace std;
void QuadPatch::generatePatch(float width, float height, unsigned int widthres, unsigned int heightres)
{
	if (width <= 0 || height <= 0 || widthres == 0 || heightres == 0)
	{
		cout << "Wrong Parameters" << endl;
		return;
	}
	mPosition.clear();
	mNormal.clear();
	mUV.clear();
	mIndex.clear();

	float widthstep = width / widthres;
	float heightstep = height / heightres;
	float widthstart = -width / 2;
	float heightstart = -height / 2;
	unsigned int index;
	for (unsigned int j = 0; j <= heightres; ++j) // for each row
	{
//		widthstart = -width / 2;
		for (unsigned int i = 0; i <= widthres; ++i)  // for each control point in each row
		{
			glm::vec4 pos(widthstart + i*widthstep, 0.0, heightstart + j*heightstep,1.0);
			for (int k = 0; k < 4; ++k)
			{
				mPosition.push_back(pos[k]);
		//		cout << pos[k] << ",";
			}
	//		cout << "     ";
			glm::vec3 nor(0.0, 1.0, 0.0);
			for (int k = 0; k < 3; ++k)
				mNormal.push_back(nor[k]);
			glm::vec2 uv(i*(1.0 / widthres), j*(1.0 / heightres));
			for (int k = 0; k < 2; ++k)
				mUV.push_back(uv[k]);
			index = j*(widthres + 1) + i;
			mIndex.push_back(index);
			//cout << index<<"  ";
		}
	//	cout << endl;
	}


}