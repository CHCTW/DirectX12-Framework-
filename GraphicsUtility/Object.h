#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
// a draw object, hold the texture id and mesh id, matrices id
class Object
{
public:
	unsigned int mMaterialID;
	unsigned int mMeshID;
	unsigned int mMatricesID;
};