#pragma once
#define GLM_ENABLE_EXPERIMENTAL 
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "stdafx.h"
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
enum FrustumPlane
{
	FRUSTUM_PLANE_LEFT,
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_TOP,
	FRUSTUM_PLANE_BOTTOM,
	FRUSTUM_PLANE_FAR,
	FRUSTUM_PLANE_NEAR,
	FRUSTUM_PLANE_COUNT
};
struct ViewProjection
{
	glm::mat4x4 mView;
	glm::mat4x4 mProjection;
	glm::vec3 mPosition;
	float padding;
	glm::mat4x4 mViewInverse;
	glm::mat4x4 mProjInverse;
	glm::vec4 mFrustumPlane[FRUSTUM_PLANE_COUNT];
	glm::mat4x4 mViewInverseTranspose;
	float front;
	float back;
	float fov;
	float ratio;
};
class Camera
{
public:
	Camera(): mFOV (45.0f),mPosition(0.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mTarget(0.0f,0.0f,3.0f), mFront(0.1f), mBack(1000.0f), mRatio(16.0f/9.0f)
	{
		mViewProjection.mView = glm::lookAt(mPosition,mTarget, mUp);
		mViewProjection.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mBack);
		mViewProjection.mProjInverse = glm::inverse(mViewProjection.mProjection);
		mViewProjection.mViewInverse = glm::inverse(mViewProjection.mView);
		mViewProjection.front = mFront;
		mViewProjection.back = mBack;
		mViewProjection.ratio = mRatio;
		mViewProjection.fov = mFOV;
		frustumPlaneCal();
	//	glm::mat4x4 test = mViewProjection.mProjection*mViewProjection.mProjInverse;
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
	void setPosition(float,float,float);
	void setTarget(float, float, float);
	void setUp(float, float, float);
	void setFOV(float);
	void setBack(float);
	void frustumPlaneCal();
	ViewProjection const * getMatrix();
	ViewProjection mViewProjection;
};