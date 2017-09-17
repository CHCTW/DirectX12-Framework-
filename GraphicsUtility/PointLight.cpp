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
	/*glm::mat4 v = glm::mat4(1.0);
	mLightData.mView = glm::translate(v, glm::vec3(mOffset, -mZoom));
	mLightData.mView = glm::rotate(mLightData.mView, (mAngle.y - 90.0f)*pi / 180, glm::vec3(1.0, 0.0, 0.0));
	mLightData.mView = glm::rotate(mLightData.mView, mAngle.x*pi / 180, glm::vec3(0.0, 1.0, 0.0));


	glm::mat4 temp;
	temp[0][0] = 0;
	temp[0][1] = 0;
	temp[0][2] = 0;
	temp[0][3] = 1;	
	temp = glm::inverse(mLightData.mView) * temp;
	mLightData.mPosition[0] = temp[0][0];
	mLightData.mPosition[1] = temp[0][1];
	mLightData.mPosition[2] = temp[0][2];*/


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
	for (int i = 0; i < attenuationCount; i++)
	{
		if (r - LightAttenuations[i].x >= 0)
		{
			mLightData.mAttenuation = glm::vec4(LightAttenuations[i], 1.0);
		}
	}
	mLightData.mRadius = r;
}
void PointLight::setColor(float r, float g, float b)
{
	mLightData.mColor = glm::vec4(r, g, b, 1.0);
}
PointLightData const * PointLight::getLightData()
{
	return &mLightData;
}

void PointLight::setPosition(float x, float y, float z)
{
	mLightData.mPosition = glm::vec4(x, y, z, 1.0);
}

