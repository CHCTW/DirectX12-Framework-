#include "stdafx.h"
#include "GameCamera.h"
#define PI 3.14159f
GameCamera::GameCamera() : Camera(), mAngle(0,90)
{
	updateViewProj();
}
void GameCamera::addAngle(float spint, float tilt)
{
	mAngle.x += spint;
	mAngle.y += tilt;
}
void GameCamera::moveFrontBack(float offset)
{
	mPosition+=glm::vec3(offset*sin(mAngle.x*PI / 180.0f), -offset*sin((mAngle.y - 90.0f)*PI / 180), -offset*cos(mAngle.x*PI / 180.0f));
}
void GameCamera::moveLeftRight(float offset)
{
	mPosition += glm::vec3(offset*cos(mAngle.x*PI / 180.0f), 0.0, offset*sin(mAngle.x*PI / 180.0f));
}
void GameCamera::moveUpDown(float offset)
{
	mPosition += glm::vec3(0.0, offset, 0.0);
}
void GameCamera::updateViewProj()
{
	mViewProjection.mView = glm::rotate(glm::mat4(1.0), (mAngle.y - 90.0f)*PI / 180.0f, glm::vec3(1.0, 0.0, 0.0));
	mViewProjection.mView = glm::rotate(mViewProjection.mView, mAngle.x*PI / 180.0f, glm::vec3(0.0, 1.0, 0.0));
	glm::vec3 temp = glm::vec3(-1.0) * mPosition;
	mViewProjection.mView = glm::translate(mViewProjection.mView, temp);
	mViewProjection.mProjection = glm::perspective(mFOV*(PI) / 180.0f, mRatio, mFront, mBack);
	mViewProjection.mPosition = mPosition;
	mViewProjection.mViewInverse = glm::inverse(mViewProjection.mView);
	mViewProjection.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mBack);
	mViewProjection.mProjInverse = glm::inverse(mViewProjection.mProjection);
	mViewProjection.mViewInverseTranspose = glm::transpose(mViewProjection.mViewInverse);
	mViewProjection.front = mFront;
	mViewProjection.back = mBack;
	frustumPlaneCal();
}