#pragma once
#include "stdafx.h"
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
struct SpotLightData
{
	glm::mat4x4 mView;
	glm::mat4x4 mProjection;
	glm::vec4 mPosition;
	glm::vec4 mColor;
	float mRadius;
	float mIntensity;
	float mConeAngle;
	float padding;
};

class SpotLight
{
public:
	SpotLight() : mFOV(90.0f), mAngle(0.0f, 90.f), mOffset(0.0, 0.0), mZoom(10.0f)
	{
		mLightData.mColor = glm::vec4(1.0f, 1.0f, 1.0f,1.0);
		mLightData.mRadius = 50.0f;
		mLightData.mIntensity = 1000.0f;
		mLightData.mConeAngle = cos(mFOV*(3.14159f) / 180.0f);
		mLightData.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, 1.0f, 0.1f, mLightData.mRadius);
		update();

	}



	glm::vec2 mAngle;
	glm::vec2 mOffset;

	float mZoom;
	void addAngle(float spint, float tilt);
	void addZoom(float zoom);
	void update();
	void setRadius(float r);
	void setIntensity(float i);
	void setConeAngle(float degree);
	void setColor(float r,float g, float b);
	SpotLightData const * getLightData();
	float mFOV;
	float mBack;



private:
	SpotLightData mLightData;
};