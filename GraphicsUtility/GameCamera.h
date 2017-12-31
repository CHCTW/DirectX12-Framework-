#pragma once
#include "stdafx.h"
//#include <glm/matrix.hpp>
////#include <glm/gtx/transform.hpp>
//#include <glm/matrix.hpp>
//#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
class GameCamera : public Camera
{
public:
	GameCamera();
	glm::vec2 mAngle;
	void addAngle(float spint, float tilt);
	void moveFrontBack(float offset);
	void moveLeftRight(float offset);
	void moveUpDown(float offset);
	void updateViewProj();
	//	float mZoomSpeed;
	//	float mRotSpeed;
};