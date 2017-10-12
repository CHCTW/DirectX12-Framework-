#pragma once
/*
Optical class is used for lens flare effect. 
The method is from the paper "Practical Real-Time Lens-Flare Rendering", Sungkil Lee, Elmar Eisemann,Eurographics Symposium on Rendering 2013
Also there is a nice turtorial on this website: https://placeholderart.wordpress.com/2015/01/19/implementation-notes-physically-based-lens-flares/

*/
struct OpticalSlice
{
	float radius;
	float distance;
	float refractindex;
};
class Optical
{

};