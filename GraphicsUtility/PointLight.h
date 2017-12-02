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
	float mRadius;
	float mIntensity;
	glm::vec2 padding;
};

class PointLight
{
public:
	PointLight() : mFOV(90.0f), mFront(0.1f), mRatio(1.0f), mAngle(0.0f, 90.f), mOffset(0.0, 0.0), mZoom(10.0f)
	{
		mLightData.mColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0);
		mLightData.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mLightData.mRadius);
		mLightData.mRadius = 100;
		mLightData.mIntensity = 1000;
		mLightData.mPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		update();
	}
	glm::vec2 mAngle;
	glm::vec2 mOffset;
	float mZoom;
	void addAngle(float spint, float tilt);
	void addZoom(float zoom);
	void update();
	void setPosition(float x,float y,float z);
	void addPosition(float x, float y, float z);
	void setRadius(float r);
	void setIntensity(float i);
	void setColor(float r, float g, float b);
	PointLightData const * getLightData();
	float mFOV;
	float mRatio;
	float mFront;
private:
	PointLightData mLightData;
};