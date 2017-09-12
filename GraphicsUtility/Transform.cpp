#include "stdafx.h"
#include "Transform.h"
#define PI 3.14159f

void Transform::CacNewTransform()
{

	mMatrices.mModel = glm::translate(mPosition);
	mMatrices.mModel = glm::rotate(mMatrices.mModel, mAngle.x*PI / 180, glm::vec3(1.0f, 0.0f, 0.0f));
	mMatrices.mModel = glm::rotate(mMatrices.mModel, mAngle.y*PI / 180, glm::vec3(0.0f, 1.0f, 0.0f));
	mMatrices.mModel = glm::rotate(mMatrices.mModel, mAngle.z*PI / 180, glm::vec3(0.0f, 0.0f, 1.0f));
	mMatrices.mModel = glm::scale(mMatrices.mModel, mScale);
	glm::mat4 mInverse = glm::inverse(mMatrices.mModel);
	mMatrices.mNormal = glm::transpose(mInverse);
}
void Transform::setPosition(glm::vec3 const & p)
{
	mPosition = p;
}
void Transform::addPosition(glm::vec3 const & p)
{
	mPosition += p;
}
void Transform::setPosition(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;
}

void Transform::addPosition(float x, float y, float z)
{
	mPosition.x += x;
	mPosition.y += y;
	mPosition.z += z;
}
void Transform::setAngle(glm::vec3 const &  ang)
{
	mAngle = ang;
}

void Transform::setAngle(float x, float y, float z)
{
	mAngle = glm::vec3(x, y, z);
}
void Transform::addAngle(glm::vec3 const & ang)
{
	mAngle += ang;
}
void Transform::setScale(glm::vec3 const &  scale)
{
	mScale = scale;
}
void Transform::setScale(float x, float y, float z)
{
	mScale = glm::vec3(x, y, z);
}
void Transform::addScale(glm::vec3 const & scale)
{
	mScale += scale;
}
Matrices const & Transform::getMatrices()
{
	return mMatrices;
}

glm::mat4 const & Transform::getModelMatrix()
{
	return mMatrices.mModel;
}
glm::vec3 const & Transform::getEulerAngle()
{
	return mAngle;
}
glm::mat4 const & Transform::getNormalMatrix()
{
	return mMatrices.mNormal;
}
glm::vec3 const & Transform::getPosition()
{
	return mPosition;
}

glm::vec3 const & Transform::getScale()
{
	return mScale;
}
