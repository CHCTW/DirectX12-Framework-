#pragma once

#include "stdafx.h"
#include "Render.h"
#include "StructureHeaders.h"
#include <glm/vec3.hpp>
// will think more about what is the positon of this class, I want to use as tool for both offline and online.
// the problem is that I don't want to record multiple same pipeline
struct WorleyNoiseConstData
{
	float scale;
	float time;
	float offsetx;
	float offsety;
	unsigned int reverse;
};

struct PerlinNoise2DConstData
{
	float frequency;
	float valuescale;
	int repeatvalue;
	float seed; // any value > 0
	float offsetx; //0~1
	float offsety; //0~1
};
struct OctaveNoise2DConstData
{
	int octaves;
	float persistance;
	float frequency;
	float valuescale;
	int repeatvalue;
	float seed; // any value > 0
	float offsetx; //0~1
	float offsety; //0~1
};
class GPUNoiseTextureGenerator
{
public:
	GPUNoiseTextureGenerator();
	~GPUNoiseTextureGenerator();
	void initialize(Render& render);
	void release();
	void recordPureGenerateWorleyNoise2D(CommandList &recordlist,Texture& texture,bool reverse = false,float scale = 1.0f,float time = 0.0f,unsigned int offsetx = 0, unsigned int offsety = 0,int miplevel = 0);
	void recordPureGeneratePerlinNoise2D(CommandList &recordlist, Texture& texture,float frequency,float time = 0.0f, float valuescale = 1.0f,int repeat = 0,float offsetx = 0, float offsety = 0, int miplevel = 0);
	void recordPureGenerateOctaveNoise2D(CommandList &recordlist, Texture& texture, float frequency,int octave = 1,float persistance = 1.0f, float time = 0.0f, float valuescale = 1.0f, int repeat = 0, float offsetx = 0, float offsety = 0, int miplevel = 0);

private:
	Pipeline mNoisePipeline[GPU_NOISE_TEXTURE_TYPE_COUNT];
	RootSignature mNoiseRootSignature[GPU_NOISE_TEXTURE_TYPE_COUNT];
};