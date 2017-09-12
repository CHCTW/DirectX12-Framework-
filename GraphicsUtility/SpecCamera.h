#pragma once
#include "stdafx.h"
#include <glm/matrix.hpp>
//#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
class SpecCamera : public Camera
{
public:
	SpecCamera::SpecCamera();
	glm::vec2 mAngle;
	glm::vec2 mOffset;
	float mZoom;
	void addAngle(float spint, float tilt);
	void addZoom(float zoom);
	void updateViewProj();
//	float mZoomSpeed;
//	float mRotSpeed;
};