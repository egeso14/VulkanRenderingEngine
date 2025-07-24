#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace VTA{
	class VTAWindow
	{
	public:
		VTAWindow(int w, int h, std::string name);
		~VTAWindow();

		VTAWindow(const VTAWindow&) = delete;
		VTAWindow& operator=(const VTAWindow&) = delete; // this is to establish unique ownership of resources

		bool shouldClose() {return glfwWindowShouldClose(window);}
		VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface); 
		
	private:
		GLFWwindow* window;
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		void initWindow();

		int width;
		int height;
		bool framebufferResized = false;

		std::string windowName;
	};
}
