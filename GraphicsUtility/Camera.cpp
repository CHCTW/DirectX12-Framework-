#include "stdafx.h"
#include "Camera.h"
void Camera::updateViewProj()
{
	mViewProjection.mView = glm::lookAt(mPosition, mTarget, mUp);
	mViewProjection.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mBack);
	mViewProjection.mProjInverse = glm::inverse(mViewProjection.mProjection);
	mViewProjection.mViewInverse = glm::inverse(mViewProjection.mView);
	mViewProjection.mPosition = mPosition;
}


ViewProjection const * Camera::getMatrix()
{
	return &mViewProjection;
}

void Camera::setRatio(float ratio)
{
	mRatio = ratio;
	mViewProjection.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mBack);
	mViewProjection.mProjInverse = glm::inverse(mViewProjection.mProjection);
}
void Camera::setPosition(float x, float y, float z)
{
	mPosition = glm::vec3(x, y, z);
}
void Camera::setTarget(float x, float y, float z)
{
	mTarget = glm::vec3(x, y, z);
}
void Camera::setUp(float x, float y, float z)
{
	mUp = glm::vec3(x, y, z);
}
void Camera::setFOV(float fov)
{
	mFOV = fov;
}