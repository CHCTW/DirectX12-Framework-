#include "stdafx.h"
#include "SpotLight.h"

void SpotLight::addAngle(float spint, float tilt)
{
	mAngle.x += spint;
	mAngle.y += tilt;
}
void SpotLight::addZoom(float zoom)
{
	mZoom += zoom;
}
void SpotLight::update()
{
	glm::mat4 v = glm::mat4(1.0);
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
	mLightData.mPosition[2] = temp[0][2];

}
void SpotLight::setRadius(float r)
{

	mLightData.mRadius = r;
	mLightData.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, 1.0f, 0.1f, mLightData.mRadius);
}
void SpotLight::setIntensity(float i)
{
	mLightData.mIntensity = i;
}
void SpotLight::setConeAngle(float degree)
{
	mLightData.mConeAngle = cos(degree*(3.14159f) / 360.0f); 
	mFOV = degree;
	mLightData.mProjection = glm::perspective(mFOV*(3.14159f) / 180.0f, 1.0f, 0.1f, mLightData.mRadius);
}
void SpotLight::setColor(float r, float g, float b)
{
	mLightData.mColor = glm::vec4(r, g, b,1.0);
}
SpotLightData const * SpotLight::getLightData()
{
	return &mLightData;
}