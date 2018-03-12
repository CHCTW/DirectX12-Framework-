#include "stdafx.h"
#include "QuadPatch.h"
#include <glm/vec4.hpp>
#include <iostream>
#include <PerlinNoise/PerlinNoise.hpp>
using namespace std;
void QuadPatch::generatePatch(float width, float height, unsigned int widthres, unsigned int heightres, IndexGenerateMode indexmode,YGenerateMode mode, unsigned int seed, float xfreqency, float yfreqency, unsigned int scale,float uscale,float vscale)
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

	glm::vec3 normal(0, 1, 0);
	glm::vec3 tangent(1, 0, 0);


	siv::PerlinNoise noise;

	for (unsigned int j = 0; j <= heightres; ++j) // for each row
	{
//		widthstart = -width / 2;
		for (unsigned int i = 0; i <= widthres; ++i)  // for each control point in each row
		{
			
	//		cout << "     ";
		//	glm::vec3 nor(0.0, 1.0, 0.0);
			for (int k = 0; k < 3; ++k)
			{
				mNormal.push_back(normal[k]);
				mTangent.push_back(tangent[k]);
			}
			glm::vec2 uv((float)i / widthres*uscale, (float)j / heightres*vscale);
		//	cout << uv.x << endl;
			for (int k = 0; k < 2; ++k)
				mUV.push_back(uv[k]);





			glm::vec4 pos(widthstart + i*widthstep, 0.0, heightstart + j*heightstep, 1.0);
			if (mode == PerlinGenerate)
			{
				//float xseed = fmod(pos.x+0.5*width, width);
				float xseed = pos.x;
				float zseed = pos.z;
			/*	if (floor(xseed) == pos.x)
					xseed += 0.01;
				if (floor(pos.z) == pos.z)
					zseed += 0.01;*/
				float temp = scale;
				pos.y = ((float)noise.noise0_1(xseed*xfreqency, zseed*yfreqency,0.5)-0.5)*2.0*temp;
				//temp = sqrt(temp);

				temp *= 0.5;

				pos.y += ((float)noise.octaveNoise0_1(xseed*xfreqency*2+temp, zseed*yfreqency*2 + temp, 1)-0.5)*2.0*temp;
				//temp = sqrt(temp);
				temp *= 0.5;
				pos.y += ((float)noise.octaveNoise0_1(xseed*xfreqency*4 + temp, zseed*yfreqency*4 + temp, 1) - 0.5)*2.0*temp;
				//temp = sqrt(temp);
				temp *= 0.5;
				pos.y += ((float)noise.octaveNoise0_1(xseed*xfreqency * 8 + temp, zseed*yfreqency * 8 + temp,  1) - 0.5)*2.0*temp;
				//temp = sqrt(temp);
				temp *= 0.5;
				pos.y += ((float)noise.octaveNoise0_1(xseed*xfreqency * 16 + temp, zseed*yfreqency * 16 + temp, 1) - 0.5)*2.0*temp;
				//cout << (float)noise.noise(uv.x, uv.y) << endl;
			}
	//		cout << pos.y << endl;
			for (int k = 0; k < 3; ++k)
			{
				mPosition.push_back(pos[k]);
				//		cout << pos[k] << ",";
			}

			//cout << index<<"  ";
		}
	
	//	cout << endl;
	}
	mIndex.clear();
	for (unsigned int j = 0; j < heightres; ++j) // for each row
	{
		//		widthstart = -width / 2;
		for (unsigned int i = 0; i < widthres; ++i)  // for each control point in each row
		{
			if (indexmode == Quad)  
			{
				mIndex.push_back(j*(widthres + 1) + i);
		//		cout << j*(widthres + 1) + i << "  ";
				mIndex.push_back(j*(widthres + 1) + i + 1);

		//		cout << j*(widthres + 1) + i + 1 << "  ";

				mIndex.push_back((j + 1)*(widthres + 1) + i);

		//		cout << (j + 1)*(widthres + 1) + i << "  ";

				mIndex.push_back((j + 1)*(widthres + 1) + i + 1);

		//		cout << (j + 1)*(widthres + 1) + i + 1 << endl;

			}
			else if (indexmode == Triangle)
			{

				mIndex.push_back(j*(widthres + 1) + i); // 0 top-left
				mIndex.push_back((j + 1)*(widthres + 1) + i); // 2 bottom-left
				mIndex.push_back(j*(widthres + 1) + i + 1); // 1 top-right



				mIndex.push_back((j + 1)*(widthres + 1) + i); // 2 bottom-left
				mIndex.push_back((j + 1)*(widthres + 1) + i + 1); // 3 bottom-right
				mIndex.push_back(j*(widthres + 1) + i + 1); // 1 top-right

			}
			else if (indexmode == SmoothBezier)  // generatre 16 points per patch
			{ // we have to take other points around each patch to gaurantee that the tri is smooth 
				for (int yoffset = -1; yoffset <= 2; ++yoffset)
				{
					for (int xoffset = -1; xoffset <= 2; ++xoffset)
					{
						int xindex = i + xoffset;
						xindex = xindex < 0 ? 0 : xindex;
						xindex = xindex > widthres ? widthres : xindex;
						int yindex = j + yoffset;
						yindex = yindex < 0 ? 0 : yindex;
						yindex = yindex > heightres ? heightres : yindex;
				//		cout << yindex*(widthres + 1) + xindex << "      ";
						mIndex.push_back(yindex*(widthres + 1) + xindex);
					}
					cout << endl;
				}
				cout << endl;
			}
			
		}
	}


}