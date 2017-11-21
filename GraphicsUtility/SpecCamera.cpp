#include "stdafx.h"
#include "SpecCamera.h"

SpecCamera::SpecCamera() : Camera(), mAngle(0.0f,90.f), mOffset(0.0, 0.0), mZoom(10.0f)
{
	updateViewProj();
}

void SpecCamera::addAngle(float spint, float tilt)
{
	mAngle.x += spint;
	mAngle.y += tilt;
}
void SpecCamera::addZoom(float zoom)
{
	mZoom += zoom;
}
void SpecCamera::updateViewProj()
{
	glm::mat4 v = glm::mat4(1.0);
	mViewProjection.mView = glm::translate(v, glm::vec3(mOffset, -mZoom));
	mViewProjection.mView = glm::rotate(mViewProjection.mView, (mAngle.y - 90.0f)*pi / 180, glm::vec3(1.0, 0.0, 0.0));
	mViewProjection.mView = glm::rotate(mViewProjection.mView, mAngle.x*pi / 180, glm::vec3(0.0, 1.0, 0.0));
	mViewProjection.mViewInverse = glm::inverse(mViewProjection.mView);
	mViewProjection.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mBack);
	mViewProjection.mProjInverse = glm::inverse(mViewProjection.mProjection);
	mViewProjection.mViewInverseTranspose = glm::transpose(mViewProjection.mViewInverse);

	glm::mat4 temp;
	temp[0][0] = 0;
	temp[0][1] = 0;
	temp[0][2] = 0;
	temp[0][3] = 1;
	temp = mViewProjection.mViewInverse * temp;
	mPosition[0] = temp[0][0];
	mPosition[1] = temp[0][1];
	mPosition[2] = temp[0][2];

	mViewProjection.mPosition = mPosition;
	mViewProjection.front = mFront;
	mViewProjection.back = mBack;
	frustumPlaneCal();
}