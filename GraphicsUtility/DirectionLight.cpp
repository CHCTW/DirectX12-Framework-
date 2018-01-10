#include "stdafx.h"
#include "DirectionLight.h"
#include "Camera.h"
#include <limits>
#include <algorithm>
#include <math.h>
#include <iostream>
#define SPLITCORRETION 0.5f

static glm::vec4 ndccornerpoints[8] = // ndc space corner point
{
	glm::vec4(-1.0f,-1.0f,1.0f,1.0f),
	glm::vec4(1.0f,-1.0f,1.0f,1.0f),
	glm::vec4(-1.0f,1.0f,1.0f,1.0f),
	glm::vec4(1.0f,1.0f,1.0f,1.0f),
	glm::vec4(-1.0f,-1.0f,0.0f,1.0f),
	glm::vec4(1.0f,-1.0f,0.0f,1.0f),
	glm::vec4(-1.0f,1.0f,0.0f,1.0f),
	glm::vec4(1.0f,1.0f,0.0f,1.0f),
};
void DirectionLight::updatewithCamera(Camera& camera)
{
	glm::vec3 eye = -glm::vec3(mData.mDirection.x*mData.mVirtualDistance, mData.mDirection.y*mData.mVirtualDistance, mData.mDirection.z*mData.mVirtualDistance);
	mData.mView = glm::lookAt(eye, glm::vec3(0.0f,0.0,0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::vec4 lightspacecornerpoints[8];
	mData.mBoxMax = glm::vec4(std::numeric_limits<float>::lowest());
	mData.mBoxMin = glm::vec4(std::numeric_limits<float>::max());
	
//	glm::mat4 view = camera.getMatrix()->mView;
//	glm::mat4 view
	glm::mat4 viewproj = camera.getMatrix()->mProjection*camera.getMatrix()->mView;
	glm::mat4 viewprojinverse = glm::inverse(viewproj);

	calLightBoundingBoxforFrus(mData.mBoxMin, mData.mBoxMax, viewprojinverse, 0.0f, 1.0f); // use ndc coordinate to caculate boudning box, near and far are in [0,1]
	mData.mProjection = glm::ortho(mData.mBoxMin.x, mData.mBoxMax.x, mData.mBoxMin.y, mData.mBoxMax.y, 0.0f, -mData.mBoxMin.z); // otho proj cover the whole view frustum
																																//	mData.mProjection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, 0.0f, -250.0f); // otho proj cover the whole view frustum
																																//	mData.mProjection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, 0.0f, -250.0f); // otho proj cover the whole view frustum
																																//	mData.mProjection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, 0.0f, -250.0f); // otho proj cover the whole view frustum
																																//	mData.mProjection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, 0.0f, -250.0f); // otho proj cover the whole view frustum


	//mData.mProjection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, 0.0f, -mData.mBoxMin.z); // otho proj cover the whole view frustum


	mData.mLightProjtoViewProj = viewproj*glm::inverse(mData.mProjection*mData.mView);



	float nearz = camera.getMatrix()->front;
	float farz = camera.getMatrix()->back;
	float prevprojz = 0.0f;
	float prevzi = nearz;
	mData.mSliceBoxMinandZ[0].w = nearz;
	for (unsigned int i = 1; i <= mData.mSliceNumber; ++i) // start to caculate 
	{
		float rate = static_cast<float>(i) / mData.mSliceNumber;
		float stdterm = SPLITCORRETION*nearz*pow(farz / nearz, rate);
		float linterm = (1.0f - SPLITCORRETION)*(nearz + rate*(farz - nearz));
		float zi = stdterm + linterm; // get depth near~far
		// start to calculate projected z 
		glm::vec4 projcoord = camera.getMatrix()->mProjection*glm::vec4(0.0, 0.0, -zi, 1.0);
		float projectz = projcoord.z/ projcoord.w;
		// use both z to caculate bounding box in light space
		calLightBoundingBoxforFrus(mData.mSliceBoxMinandZ[i-1], mData.mSliceBoxMaxandZ[i-1], viewprojinverse, prevprojz, projectz);
		glm::vec4 center;

		

		float rad;


		calLightBoundingSphereforFrus(center, rad, camera.getMatrix()->mViewInverse, prevzi, zi, camera.getMatrix()->ratio,camera.getMatrix()->fov);
		

		glm::vec2 offset(rad*2 / 512.0f, rad*2 / 512.0f); // offset for fix pixel
		glm::vec2 rem(-fmod(center.x,offset.x), -fmod(center.y, offset.y));
		center.x += rem.x;
		center.y += rem.y;


		//cout << (mData.mSliceBoxMinandZ[i - 1].x + mData.mSliceBoxMaxandZ[i - 1].x) / 2.0f << endl;
		//cout << rad << endl;
		



//		mData.mSliceProjection[i - 1] = glm::ortho(mData.mSliceBoxMinandZ[i - 1].x, mData.mSliceBoxMaxandZ[i - 1].x, mData.mSliceBoxMinandZ[i - 1].y, mData.mSliceBoxMaxandZ[i - 1].y, 0.0f, -mData.mSliceBoxMinandZ[i - 1].z);

		mData.mSliceProjection[i - 1] = glm::ortho(center.x -rad, center.x+rad, center.y-rad, center.y+rad,0.0f, -center.z+rad);





		mData.mSliceLightProjtoViewProj[i - 1] = viewproj*glm::inverse(mData.mSliceProjection[i - 1] *mData.mView);
		prevprojz = projectz;
		prevzi = zi;
	}
	//camera.getMatrix.mViewProjection;
}
void DirectionLight::calLightBoundingBoxforFrus(glm::vec4& min, glm::vec4& max, glm::mat4x4& camviewprojinv, float near, float far)
{


	max = glm::vec4(std::numeric_limits<float>::lowest());
	min = glm::vec4(std::numeric_limits<float>::max());
	glm::vec4 lightspacepoint;

	for (int i = 0; i < 8; ++i)
	{
		if (i <= 3) // first 4 use near z value
			ndccornerpoints[i].z = near;
		else
			ndccornerpoints[i].z = far;
		lightspacepoint = camviewprojinv*ndccornerpoints[i]; // corner point in view space
		lightspacepoint = mData.mView*lightspacepoint;
		lightspacepoint = lightspacepoint / lightspacepoint.w;

		max.x = std::max(max.x, lightspacepoint.x);
		max.y = std::max(max.y, lightspacepoint.y);
		max.z = std::max(max.z, lightspacepoint.z);
		min.x = std::min(min.x, lightspacepoint.x);
		min.y = std::min(min.y, lightspacepoint.y);
		min.z = std::min(min.z, lightspacepoint.z);
	}
	min.w = near;
	max.w = far;

}
void DirectionLight::calLightBoundingSphereforFrus(glm::vec4& center, float& radius, glm::mat4x4 const & camviewinv, float near, float far,float ratio,float fov)
{
	glm::vec4 wspacepoint[8];
	float k = sqrt(1 + (ratio*ratio))*tan(fov*3.14159f /360.0f);

	if (k*k >= (far - near) / (far + near))
	{
		center.x = 0.0f;
		center.y = 0.0f;
		center.z = -far;
		center.w = 1.0;
		radius = far*k;
	}
	else
	{
		center.x = 0.0f;
		center.y = 0.0f;
		center.z = -0.5f*(far+near)*(1+k*k);
		center.w = 1.0;
		float t1 = pow((far - near), 2);
		float t2 = 2 * (far*far + near*near)*k*k;
		float t3 = pow((far + near), 2)*pow(k, 4);
		radius = 0.5f*sqrt(t1 + t2 + t3);
	}

	center = camviewinv*center;
//	cout << center.x << "  " << center.y << "   " << center.z << endl;
	center = mData.mView*center; // move center to light space
//	cout << center.x << "  " << center.y << "   " << center.z << "   " << center.w << endl;
}