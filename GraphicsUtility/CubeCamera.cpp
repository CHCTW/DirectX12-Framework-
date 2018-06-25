#include "stdafx.h"
#include "CubeCamera.h"
CubeCamera::CubeCamera()
{
	mCubeViewProjection.front = 0.1f;
	mCubeViewProjection.back = 1000.0f;
	mCubeViewProjection.mPosition = glm::vec4(0.0f, 0.0f, 0.0f,1.0f);
	updateViewProj();

}
void CubeCamera::updateViewProj()
{
	glm::vec3 temp;
	temp.x = mCubeViewProjection.mPosition.x;
	temp.y = mCubeViewProjection.mPosition.y;
	temp.z = mCubeViewProjection.mPosition.z;

	// due to cube face in dx is still useing left hand rule, building the view projection matrix for it become a little tricky
	mCubeViewProjection.mView[CUBE_FACE_ORDER_POSITIVE_X] = glm::lookAtLH(temp, temp + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	mCubeViewProjection.mView[CUBE_FACE_ORDER_NEGATIVE_X] = glm::lookAtLH(temp, temp + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	mCubeViewProjection.mView[CUBE_FACE_ORDER_POSITIVE_Y] = glm::lookAt(temp, temp + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));//z is correct
	mCubeViewProjection.mView[CUBE_FACE_ORDER_NEGATIVE_Y] = glm::lookAt(temp, temp + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	mCubeViewProjection.mView[CUBE_FACE_ORDER_POSITIVE_Z] = glm::lookAtLH(temp, temp + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));
	mCubeViewProjection.mView[CUBE_FACE_ORDER_NEGATIVE_Z] = glm::lookAtLH(temp, temp + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0));
	mCubeViewProjection.mProjection = glm::perspectiveLH(90.0f*(3.14159f) / 180.0f, 1.0f, mCubeViewProjection.front, mCubeViewProjection.back);

	for (int i = 0; i < CUBE_FACE_ORDER_COUNT; ++i)
	{
		mCubeViewProjection.mProjectionView[i] = mCubeViewProjection.mProjection*mCubeViewProjection.mView[i];
		mCubeViewProjection.mProjectionViewInverse[i] = glm::inverse(mCubeViewProjection.mProjectionView[i]);
	}
	mUpdated = true;
}
void CubeCamera::setPosition(float x, float y, float z)
{
	mCubeViewProjection.mPosition = glm::vec4(x, y, z, 1.0f);
	mUpdated = false;
}
CubeViewProjection const * CubeCamera::getMatrix()
{
	if (!mUpdated)
		updateViewProj();
	return &mCubeViewProjection;
}
void CubeCamera::setBack(float back)
{
	mCubeViewProjection.back = back;
	mUpdated = false;
}