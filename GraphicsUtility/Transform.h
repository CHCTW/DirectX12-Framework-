#pragma once
#include "stdafx.h"
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
struct Matrices
{
	Matrices(): mModel(1), mNormal(1)
	{

	}
	glm::mat4 mModel;
	glm::mat4 mNormal;
};
class Transform
{
public:
	Transform() :mPosition(0), mAngle(0), mScale(1) {}
	~Transform() {}
	Matrices const & getMatrices();
	glm::mat4 const & getModelMatrix();
	glm::mat4 const & getNormalMatrix();
	glm::vec3 const & getPosition();
	glm::vec3 const & getEulerAngle();
	glm::vec3 const & getScale();
	void setPosition(glm::vec3 const & p);
	void addPosition(glm::vec3 const & p);
	void setPosition(float x, float y, float z);
	void addPosition(float x, float y, float z);
	void setAngle(glm::vec3 const & ang);
	void setAngle(float x, float y, float z);
	void addAngle(glm::vec3 const & ang);
	void setScale(glm::vec3 const & scale);
	void setScale(float x, float y, float z);
	void addScale(glm::vec3 const & scale);
	void CacNewTransform();
	glm::vec3 mPosition;
	glm::vec3 mAngle;
	glm::vec3 mScale;
	Matrices mMatrices;
};