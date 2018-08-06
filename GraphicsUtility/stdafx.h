// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers


// TODO: reference additional headers your program requires here
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <glm/vec3.hpp>
using namespace std;
static const int attenuationCount = 12;
static glm::vec3 LightAttenuations[attenuationCount] =
{
	glm::vec3(7.0f,0.7f,1.8f),
	glm::vec3(13.0f,0.35f,0.44f),
	glm::vec3(20.0f,0.22f,0.2f),
	glm::vec3(32.0f,0.14f,0.07f),
	glm::vec3(50.0f, 0.09f,0.032f),
	glm::vec3(65.0f, 0.07f,0.017f),
	glm::vec3(100.0f, 0.045f, 0.0075f),
	glm::vec3(160.0f, 0.027f, 0.0028f),
	glm::vec3(200.0f, 0.022f, 0.0019f),
	glm::vec3(325.0f, 0.014f, 0.0007f),
	glm::vec3(600.0f, 0.007f, 0.0002f),
	glm::vec3(3250.0f, 0.0014f, 0.000007f)

};
static float pi = 3.1415f;

enum FrustumPlane
{
	FRUSTUM_PLANE_LEFT,
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_TOP,
	FRUSTUM_PLANE_BOTTOM,
	FRUSTUM_PLANE_FAR,
	FRUSTUM_PLANE_NEAR,
	FRUSTUM_PLANE_COUNT
};

enum CubeFaceOrder
{
	CUBE_FACE_ORDER_POSITIVE_X = 0,
	CUBE_FACE_ORDER_NEGATIVE_X = 1,
	CUBE_FACE_ORDER_POSITIVE_Y = 2,
	CUBE_FACE_ORDER_NEGATIVE_Y = 3,
	CUBE_FACE_ORDER_POSITIVE_Z = 4,
	CUBE_FACE_ORDER_NEGATIVE_Z = 5,
	CUBE_FACE_ORDER_COUNT
};


enum GPUNoiseTextureType
{
	GPU_NOISE_TEXTURE_TYPE_2D_WORLEY,
	GPU_NOISE_TEXTURE_TYPE_2D_PERLIN,
	GPU_NOISE_TEXTURE_TYPE_2D_OCTAVE,
	GPU_NOISE_TEXTURE_TYPE_COUNT
};


const static char *NoiseTextureShadersName[GPU_NOISE_TEXTURE_TYPE_COUNT]
{
	"WorleyNoise2D.hlsl",
	"PerlinNoise2D.hlsl",
	"OctaveNoise2D.hlsl"
};