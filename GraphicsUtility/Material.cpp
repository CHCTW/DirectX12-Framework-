#include "stdafx.h"
#include "Material.h"
void Material::setNormalMapIndex(unsigned int i)
{
	mTextureIndex[MATERIALMAP_INDEX_NORMAL] = i;
	mChoose[MATERIALMAP_INDEX_NORMAL] = 0.0f;
}
void Material::setColorMapIndex(unsigned int i)
{
	mTextureIndex[MATERIALMAP_INDEX_COLOR] = i;
	mChoose[MATERIALMAP_INDEX_COLOR] = 0.0f;
}
void Material::setRoughnessMapIndex(unsigned int i)
{
	mTextureIndex[MATERIALMAP_INDEX_ROUGHNESS] = i;
	mChoose[MATERIALMAP_INDEX_ROUGHNESS] = 0.0f;
}
void Material::setMetalicMapIndex(unsigned int i)
{
	mTextureIndex[MATERIALMAP_INDEX_METALIC] = i;
	mChoose[MATERIALMAP_INDEX_METALIC] = 0.0f;
}
void Material::setColor(float r, float g, float b)
{
	mAlbedo.x = r;
	mAlbedo.y = g;
	mAlbedo.z = b;
	mChoose[MATERIALMAP_INDEX_COLOR] = 1.0f;
}
void Material::setRoughness(float r)
{
	mRoughness = r;
	mChoose[MATERIALMAP_INDEX_ROUGHNESS] = 1.0f;
}
void Material::setMetalic(float m)
{
	mMetallic = m;
	mChoose[MATERIALMAP_INDEX_METALIC] = 1.0f;
}