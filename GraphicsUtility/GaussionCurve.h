#pragma once
#include "stdafx.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <math.h>
using namespace std;
#define PI 3.14159f
#define MAXRADIUS 50u
struct GaussionWeight
{
	unsigned int radius;
	float padding1;
	float padding2;
	float padding3;
	float weight[MAXRADIUS * 2 + 1];
};
class GaussionCurve
{
public:
	GaussionCurve(float ra, float sig, float mean = 0.0f) : radius(ra), mean(mean), sigma(sig) // defulat approixmiate 1 to sample one data
	{
		radius = std::min(radius, MAXRADIUS);
		
	}
	GaussionWeight generateNormalizeWeight()
	{
		GaussionWeight res;
		unsigned int samplenum = radius;
		samplenum *= 2;
		samplenum += 1;
		vector<float> data(samplenum);
		float coef = 1.0 / (sigma*sqrt(2 * PI));
		float x = -(float)radius;  // start with most left point
		float offset = (float)radius * 2.0f / (float)(samplenum-1);
		float acc = 0.0f;
		for (unsigned int i = 0; i < samplenum; ++i)
		{
			
			float temp = (x - mean) / sigma;
			temp *= temp;
			data[i] = exp(-0.5f*temp);
			acc += data[i];
			x += offset;
		}
		for (unsigned int i = 0; i < samplenum; ++i)
		{
			data[i] /= acc;
			res.weight[i] = data[i];
			cout << data[i] << endl;
		}
		res.radius = radius;
		return res;
		//return data;
	}
	void setRadiusandUpdateSample(float r)
	{
		radius = r;
		radius = std::min(radius, MAXRADIUS);
	}
	unsigned int radius;
	float  mean;
	float sigma;
	//vector<float> data;
};