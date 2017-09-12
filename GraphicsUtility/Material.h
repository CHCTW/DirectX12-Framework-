#pragma once
#include "stdafx.h"
#include <glm/vec3.hpp>
enum MaterialType
{

};
class Material
{
public:
	Material() : mRoughness(0.8), mMetallic(0.0), mAlbedo(0.5)
	{

	}
	float mRoughness;
	float mMetallic;
	glm::vec3 mAlbedo;
};