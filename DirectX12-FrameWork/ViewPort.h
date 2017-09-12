#pragma once
#include "d3dx12.h"
class ViewPort
{
public:
	void setup(float topleftX, float topleftY, float width, float height);
	CD3DX12_VIEWPORT mViewPort;
};