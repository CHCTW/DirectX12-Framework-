#include "Window.h"
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}
Window::Window() : mWidth(0),mHeight(0),mWindow(NULL),mIsOpen(false)
{

}
bool Window::initialize(uint32_t width, uint32_t height, std::string name, bool fullscreen)
{
	mWidth = width;
	mHeight = height;
	mName = name;
	mFullScreen = fullscreen;
	if (!glfwInit())
	{
		std::cout << "Fail to inialize" << std::endl;
	}
	return true;
}
bool Window::openWindow()
{
	if(!mFullScreen)
		mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), NULL, NULL);
	else
		mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), glfwGetPrimaryMonitor(), NULL);
	if (mWindow)
	{
		glfwSetKeyCallback(mWindow, key_callback);
		mIsOpen = true;
		return true;
	}
	return false;
}

void Window::closeWindow()
{
	glfwDestroyWindow(mWindow);
}
void Window::termianate()
{
	glfwTerminate();
}
bool Window::isRunning()
{
	return !glfwWindowShouldClose(mWindow);
}
void Window::pollInput()
{
	
	glfwPollEvents();
}

void Window::setTitle(std::string title)
{
	glfwSetWindowTitle(mWindow, title.c_str());
}