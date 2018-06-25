#pragma once
#include "stdafx.h"
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
struct CubeViewProjection
{
	glm::mat4x4 mView[CubeFaceOrder::CUBE_FACE_ORDER_COUNT];
	glm::mat4x4 mProjection;
	glm::mat4x4 mProjectionView[CubeFaceOrder::CUBE_FACE_ORDER_COUNT];
	glm::mat4x4 mProjectionViewInverse[CubeFaceOrder::CUBE_FACE_ORDER_COUNT];
	glm::vec4 mPosition;
	float front;
	float back;
	glm::vec2 padding;
};
class CubeCamera
{
public:
	CubeCamera();
	void setPosition(float x, float y, float z);
	CubeViewProjection const * getMatrix();
	void setBack(float);
private:
	bool mUpdated;
	void updateViewProj();
	CubeViewProjection mCubeViewProjection;
};