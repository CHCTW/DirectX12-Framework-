#pragma once
#include "stdafx.h"
#include "glm\vec4.hpp"
#include <vector>
#include <assimp\ai_assert.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\mesh.h>
class Mesh 
{
public:
	std::vector<glm::vec3> vertex;
	std::vector<glm::vec2> uv;
	std::vector<glm::vec3> normal;
	std::vector<glm::vec3> tangent;
	std::vector<unsigned int> indecis;
};