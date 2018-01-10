#pragma once
#include "stdafx.h"
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define MAX_CASCADENUM 8
class Camera;
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
struct DirectionLightData
{
	glm::mat4x4 mView; 
	glm::mat4x4 mProjection; // projection to cover all view frustum
	glm::mat4x4 mLightProjtoViewProj;
	glm::vec4 mBoxMax;  // a bounding box for othogoanl projection, also this store the z value 
	glm::vec4 mBoxMin;
	glm::mat4x4 mSliceProjection[MAX_CASCADENUM]; // maximum projection for cascade shadow map
	glm::mat4x4 mSliceLightProjtoViewProj[MAX_CASCADENUM];
	glm::vec4 mSliceBoxMaxandZ[MAX_CASCADENUM];  // a bounding box for othogoanl projection for each slice
	glm::vec4 mSliceBoxMinandZ[MAX_CASCADENUM];
	glm::vec4 mDirection; 
	glm::vec4 mColor;
	float mVirtualDistance;
	float mIntensity;
	unsigned int mSliceNumber;
	float padding;
};
class DirectionLight
{
public:
	DirectionLight() 
	{
		mData.mDirection = glm::normalize(glm::vec4(-0.3, -1.0, -1.0, 0.0));
		mData.mColor = glm::vec4(1.0, 0.45, 0.1, 0.0);
		mData.mIntensity = 15.0f;
		mData.mSliceNumber = 3;
		mData.mVirtualDistance = 250.0f;
	};
	void setDirection(float x,float y, float z)
	{
		mData.mDirection = glm::normalize(glm::vec4(x, y, z, 0.0));
	}
	void setSliceNumber(unsigned int n)
	{
		mData.mSliceNumber = n;
	}
	void setColor(float r, float g, float b)
	{
		mData.mColor = glm::vec4(r, g, b, 0.0f);
	}
	void setIntensity(float intensity)
	{
		mData.mIntensity = intensity;
	}
	void updatewithCamera(Camera& camera);
	DirectionLightData* getData()
	{
		return &mData;
	}
	DirectionLightData mData;
private:
	void calLightBoundingBoxforFrus(glm::vec4& min, glm::vec4& max, glm::mat4x4& viewproj, float near, float far);
	void calLightBoundingSphereforFrus(glm::vec4& center, float& radius, glm::mat4x4 const & camviewinv, float near, float far, float ratio, float fov);

};