#include "ViewPort.h"

void ViewPort::setup(float topleftX, float topleftY, float width, float height)
{
	mViewPort.TopLeftX = topleftX;
	mViewPort.TopLeftY = topleftY;
	mViewPort.Width = width;
	mViewPort.Height = height;
	mViewPort.MinDepth = 0.0f;
	mViewPort.MaxDepth = 1.0f;
}