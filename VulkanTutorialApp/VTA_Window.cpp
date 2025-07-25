#include "VTA_Window.h"
#include <stdexcept>

namespace VTA
{
	VTAWindow::VTAWindow(int w, int h, std::string name)
		: width(w), height(h), windowName(name)
	{
		initWindow();
	}
	VTAWindow::~VTAWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	void VTAWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}
	void VTAWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto vtaWIndow = reinterpret_cast<VTAWindow*>(glfwGetWindowUserPointer(window));
		vtaWIndow->framebufferResized = true;
		vtaWIndow->width = width;
		vtaWIndow->height = height;
	}
	void VTAWindow::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); 
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback); // this will call the callback function when the window is resized

	}
}