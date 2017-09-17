#pragma once
#include "stdafx.h"
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
struct PointLightData
{
	glm::mat4x4 mView[6];
	glm::mat4x4 mProjection;
	glm::vec4 mPosition;
	glm::vec4 mColor;
	glm::vec4 mAttenuation;
	float mRadius;
	float mBack;
	glm::vec2 padding;
};

class PointLight
{
public:
	PointLight() : mFOV(90.0f), mFront(0.1f), mBack(500.0f), mRatio(1.0f), mAngle(0.0f, 90.f), mOffset(0.0, 0.0), mZoom(10.0f)
	{
		mLightData.mColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0);
		//	mLightData.mPosition = glm::vec3(10.0, 10.0, 0.0);
		mLightData.mAttenuation = glm::vec4(LightAttenuations[6], 1.0);
		mLightData.mRadius = LightAttenuations[6].x;
		mLightData.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mBack);
		mLightData.mBack = mBack;

		update();

	}



	glm::vec2 mAngle;
	glm::vec2 mOffset;

	float mZoom;
	void addAngle(float spint, float tilt);
	void addZoom(float zoom);
	void update();
	void setPosition(float x,float y,float z);
	void setRadius(float r);
	void setColor(float r, float g, float b);
	PointLightData const * getLightData();
	float mFOV;
	float mRatio;
	float mFront;
	float mBack;



private:
	PointLightData mLightData;
};