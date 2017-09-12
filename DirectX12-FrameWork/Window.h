#pragma once
#define GLFW_EXPOSE_NATIVE_WIN32
#include <iostream>
#include <string>
#include "GLFW\glfw3.h"
#include "GLFW\glfw3native.h"
#pragma comment( lib, "glfw3")
class Window
{
public:
	Window();
	bool initialize(uint32_t width,uint32_t height,std::string name,bool fullscreen = false);
	bool openWindow();
	void closeWindow();
	void termianate();
	void setTitle(std::string title);
	bool isRunning();
	void pollInput();
	bool mIsOpen;
	bool mFullScreen;
	uint32_t mWidth;
	uint32_t mHeight;
	std::string mName;
	GLFWwindow* mWindow;
};