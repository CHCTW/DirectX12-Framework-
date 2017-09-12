#pragma once
#include "d3dx12.h"
class Scissor
{
public:
	void setup(UINT left, UINT right, UINT top, UINT bottom);
	CD3DX12_RECT mScissorRect;
};