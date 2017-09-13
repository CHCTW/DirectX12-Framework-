#pragma once
#define GLM_ENABLE_EXPERIMENTAL 
#include "stdafx.h"
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

struct ViewProjection
{
	glm::mat4x4 mView;
	glm::mat4x4 mProjection;
	glm::vec3 mPosition;
	float padding;
	glm::mat4x4 mProjInverse;
};
class Camera
{
public:
	Camera::Camera(): mFOV (45.0f),mPosition(0.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mTarget(0.0f,0.0f,3.0f), mFront(0.1f), mBack(1000.0f), mRatio(16.0f/9.0f)
	{
		mViewProjection.mView = glm::lookAt(mTarget, mPosition, mUp);
		mViewProjection.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mBack);
		mViewProjection.mProjInverse = glm::inverse(mViewProjection.mProjection);

		glm::mat4x4 test = mViewProjection.mProjection*mViewProjection.mProjInverse;
	}
	void setRatio(float ratio);
	float mFOV;
	float mRatio;
	float mFront;
	float mBack;
	glm::vec3 mPosition;
	glm::vec3 mTarget;
	glm::vec3 mUp;
	virtual void updateViewProj();
	ViewProjection const * getMatrix();
	ViewProjection mViewProjection;
};