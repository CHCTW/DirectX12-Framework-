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
	for (int i = 0; i < attenuationCount; i++)
	{
		if (r - LightAttenuations[i].x >= 0)
		{
			mLightData.mAttenuation = glm::vec4(LightAttenuations[i],1.0);
		}
	}
	mLightData.mRadius = r;
}
void SpotLight::setColor(float r, float g, float b)
{
	mLightData.mColor = glm::vec4(r, g, b,1.0);
}
SpotLightData const * SpotLight::getLightData()
{
	return &mLightData;
}