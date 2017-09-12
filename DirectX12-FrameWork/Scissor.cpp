#include "Scissor.h"
void Scissor::setup(UINT left, UINT right, UINT top, UINT bottom)
{
	mScissorRect.left = left;
	mScissorRect.right = right;
	mScissorRect.top = top;
	mScissorRect.bottom = bottom;
}