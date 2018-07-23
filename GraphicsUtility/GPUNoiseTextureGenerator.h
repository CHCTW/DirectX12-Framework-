#pragma once

#include "stdafx.h"
#include "Render.h"
#include "StructureHeaders.h"
#include <glm/vec3.hpp>
// will think more about what is the positon of this class, I want to use as tool for both offline and online.
// the problem is that I don't want to record multiple same pipeline
struct NoiseConstData
{
	float scale;
	float time;
	float offsetx;
	float offsety;
	unsigned int reverse;
};
class GPUNoiseTextureGenerator
{
public:
	GPUNoiseTextureGenerator();
	~GPUNoiseTextureGenerator();
	void initialize(Render& render);
	void release();
	void recordPureGenerateWorleyNoise2D(CommandList &recordlist,Texture& texture,bool reverse = false,float scale = 1.0f,float time = 0.0f,unsigned int offsetx = 0, unsigned int offsety = 0,int miplevel = 0);
private:
	Pipeline mNoisePipeline[GPU_NOISE_TEXTURE_TYPE_COUNT];
	RootSignature mNoiseRootSignature[GPU_NOISE_TEXTURE_TYPE_COUNT];
};