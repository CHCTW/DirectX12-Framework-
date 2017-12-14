#include "stdafx.h"
#include "PointLight.h"

void PointLight::addAngle(float spint, float tilt)
{
	mAngle.x += spint;
	mAngle.y += tilt;
}
void PointLight::addZoom(float zoom)
{
	mZoom += zoom;
}
void PointLight::update()
{



	//glm::vec3 test;
	mLightData.mPosition.x = mZoom*sin((mAngle.x)*pi / 180)*cos((mAngle.y - 90)*pi / 180);
	mLightData.mPosition.y = mZoom*sin((mAngle.y - 90)*pi / 180);
	mLightData.mPosition.z = mZoom*cos(mAngle.x*pi / 180)*cos((mAngle.y - 90)*pi / 180);

	glm::vec3 temp;
	temp.x = mLightData.mPosition.x;
	temp.y = mLightData.mPosition.y;
	temp.z = mLightData.mPosition.z;


	mLightData.mView[0] = glm::lookAt(temp, temp + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	mLightData.mView[1] = glm::lookAt(temp, temp + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));

	mLightData.mView[2] = glm::lookAt(temp, temp + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)); //z is correct
	mLightData.mView[3] = glm::lookAt(temp, temp + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));

	mLightData.mView[4] = glm::lookAt(temp, temp + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
	mLightData.mView[5] = glm::lookAt(temp, temp + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));



}
void PointLight::setRadius(float r)
{
	mLightData.mRadius = r;
	mLightData.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, 1.0f, 0.01f, mLightData.mRadius);
}

void PointLight::setIntensity(float i)
{
	mLightData.mIntensity = i;
}
void PointLight::setColor(float r, float g, float b)
{
	mLightData.mColor = glm::vec4(r, g, b, 1.0);
}
PointLightData const * PointLight::getLightData()
{
	glm::vec3 temp = glm::vec3(mLightData.mPosition.x, mLightData.mPosition.y, mLightData.mPosition.z);
		mLightData.mView[0] = glm::lookAt(temp, temp + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	mLightData.mView[1] = glm::lookAt(temp, temp + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));

	mLightData.mView[2] = glm::lookAt(temp, temp + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)); //z is correct
	mLightData.mView[3] = glm::lookAt(temp, temp + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));

	mLightData.mView[4] = glm::lookAt(temp, temp + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
	mLightData.mView[5] = glm::lookAt(temp, temp + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));

	return &mLightData;
}

void PointLight::setPosition(float x, float y, float z)
{
	mLightData.mPosition = glm::vec4(x, y, z, 1.0);
}

void PointLight::addPosition(float x, float y, float z)
{
	mLightData.mPosition += glm::vec4(x, y, z, 0.0);
}