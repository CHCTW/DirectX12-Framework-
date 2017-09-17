#include "stdafx.h"
#include "Camera.h"
void Camera::updateViewProj()
{
	mViewProjection.mView = glm::lookAt(mTarget, mPosition, mUp);
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