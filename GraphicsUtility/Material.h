#pragma once
#include "stdafx.h"
#include <glm/vec3.hpp>
enum MaterialMapIndex
{
	MATERIALMAP_INDEX_COLOR,
	MATERIALMAP_INDEX_NORMAL,
	MATERIALMAP_INDEX_ROUGHNESS,
	MATERIALMAP_INDEX_METALIC,
	MATERIALMAP_INDEX_COUNT
};
class Material
{
public:
	Material() : mRoughness(0.8), mMetallic(0.0), mAlbedo(0.5)
	{
		mTextureIndex[MATERIALMAP_INDEX_NORMAL] = 0;
		mTextureIndex[MATERIALMAP_INDEX_COLOR] = 0;
		mTextureIndex[MATERIALMAP_INDEX_ROUGHNESS] = 0;
		mTextureIndex[MATERIALMAP_INDEX_METALIC] = 0;

		mChoose[MATERIALMAP_INDEX_NORMAL] = 1.0f;
		mChoose[MATERIALMAP_INDEX_COLOR] = 1.0f;
		mChoose[MATERIALMAP_INDEX_ROUGHNESS] = 1.0f;
		mChoose[MATERIALMAP_INDEX_METALIC] = 1.0f;
	}

	void setNormalMapIndex(unsigned int i);
	void setColorMapIndex(unsigned int i);
	void setRoughnessMapIndex(unsigned int i);
	void setMetalicMapIndex(unsigned int i);
	void setColor(float r,float g,float b);
	void setRoughness(float r);
	void setMetalic(float m);
	// pure value, going to use it if no mapping
	float mRoughness; 
	float mMetallic;
	glm::vec3 mAlbedo;
	// texture index; 
	//  0 :base color map 
	//  1 : normal map, 
	//  2 : roughness map 
	//  3 : metalic map
	unsigned int mTextureIndex[MATERIALMAP_INDEX_COUNT];
	// combiniation choose between parameters and data from textures
	// the choose should be normal, albedeo, roughness, metalic
	// will act like roughness*mChoose[3] + texturearray[mTextureIndex[3]]*(1-mChoose[3])
	// normal map act :¡@normal*mChoose[0] + calnormal(texturearray[mTextureIndex[0]])*(1-mChoose[0])
	// two things should be noticed:
	// in this way to calcuate normals, I don't need to have a default normal map, even if the normal map is incorret.
	// since the value if never going use, it is ok to use any texture
	// other good ways, always have a default normal map with value (0.0,1.0,0.0), with size 1*1, then the choose is uneeded

	float mChoose[MATERIALMAP_INDEX_COUNT];
};