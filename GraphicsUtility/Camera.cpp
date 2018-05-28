#include "stdafx.h"
#include "Camera.h"
#include <glm/glm.hpp> 
#include <iostream>
using namespace std;
void Camera::updateViewProj()
{
	mViewProjection.mView = glm::lookAt(mPosition, mTarget, mUp);
	mViewProjection.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, mRatio, mFront, mBack);
	mViewProjection.mProjectionView = mViewProjection.mProjection*mViewProjection.mView;
	mViewProjection.mProjectionViewInverse = glm::inverse(mViewProjection.mProjectionView);

	mViewProjection.mProjInverse = glm::inverse(mViewProjection.mProjection);
	mViewProjection.mViewInverse = glm::inverse(mViewProjection.mView);
	mViewProjection.mPosition = mPosition;
	mViewProjection.front = mFront;
	mViewProjection.back = mBack;
	mViewProjection.fov = mFOV;
	mViewProjection.ratio = mRatio;
	mViewProjection.mViewInverseTranspose = glm::transpose(mViewProjection.mViewInverse);
	frustumPlaneCal();
}
void Camera::frustumPlaneCal()
{
	glm::mat4 combined = mViewProjection.mProjection*mViewProjection.mView;
	glm::vec3 temp;
	float length;

	temp.x = combined[0][3] + combined[0][0];
	temp.y = combined[1][3] + combined[1][0];
	temp.z = combined[2][3] + combined[2][0];
	
	length = temp.length();
	//temp = glm::normalize(temp);

	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_LEFT].x = temp.x/ length;
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_LEFT].y = temp.y/ length;
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_LEFT].z = temp.z/ length;
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_LEFT].w = (combined[3][3] + combined[3][0])/ length;



	temp.x = combined[0][3] - combined[0][0];
	temp.y = combined[1][3] - combined[1][0];
	temp.z = combined[2][3] - combined[2][0];

	length = temp.length();


	
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_RIGHT].x = combined[0][3] - combined[0][0];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_RIGHT].y = combined[1][3] - combined[1][0];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_RIGHT].z = combined[2][3] - combined[2][0];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_RIGHT].w = combined[3][3] - combined[3][0];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_RIGHT] /= length;






	temp.x = combined[0][3] - combined[0][1];
	temp.y = combined[1][3] - combined[1][1];
	temp.z = combined[2][3] - combined[2][1];

	length = temp.length();


	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].x = combined[0][3] - combined[0][1];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].y = combined[1][3] - combined[1][1];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].z = combined[2][3] - combined[2][1];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].w = combined[3][3] - combined[3][1];
	
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP] /= length;

	//glm::vec3 temp;
	//temp.x = mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].x;
	//temp.y = mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].y;
	//temp.z = mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].z;
	//temp = glm::normalize(temp);
	//mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].x = temp.x;
	//mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].y = temp.y;
	//mViewProjection.mFrustumPlane[FRUSTUM_PLANE_TOP].z = temp.z;




	temp.x = combined[0][3] + combined[0][1];
	temp.y = combined[1][3] + combined[1][1];
	temp.z = combined[2][3] + combined[2][1];

	length = temp.length();


	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_BOTTOM].x = combined[0][3] + combined[0][1];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_BOTTOM].y = combined[1][3] + combined[1][1];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_BOTTOM].z = combined[2][3] + combined[2][1];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_BOTTOM].w = combined[3][3] + combined[3][1];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_BOTTOM] /= length;


	temp.x = combined[0][3] + combined[0][2];
	temp.y = combined[1][3] + combined[1][2];
	temp.z = combined[2][3] + combined[2][2];

	length = temp.length();



	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_NEAR].x = combined[0][3] + combined[0][2];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_NEAR].y = combined[1][3] + combined[1][2];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_NEAR].z = combined[2][3] + combined[2][2];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_NEAR].w = combined[3][3] + combined[3][2];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_NEAR] /= length;



	temp.x = combined[0][3] - combined[0][2];
	temp.y = combined[1][3] - combined[1][2];
	temp.z = combined[2][3] - combined[2][2];

	length = temp.length();

	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_FAR].x = combined[0][3] - combined[0][2];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_FAR].y = combined[1][3] - combined[1][2];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_FAR].z = combined[2][3] - combined[2][2];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_FAR].w = combined[3][3] - combined[3][2];
	mViewProjection.mFrustumPlane[FRUSTUM_PLANE_FAR] /= length;
}


ViewProjection const * Camera::getMatrix()
{
	return &mViewProjection;
}

void Camera::setRatio(float ratio)
{
	mRatio = ratio;
	mViewProjection.ratio = mRatio;
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
	mViewProjection.fov = mFOV;
}
void Camera::setBack(float f)
{
	mBack = f;
	mViewProjection.back = mBack;
}