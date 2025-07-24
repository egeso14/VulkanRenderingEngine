#include "AppControl.h"
#include "simple_render_system.h"
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace VTA
{ 


	AppControl::AppControl()
	{
		loadGameObjects(); // load the model data into memory

	}

	AppControl::~AppControl()
	{
		
	}

	void AppControl::run()
	{
		SimpleRenderSystem simpleRenderSystem{ device, renderer.getSwapChainRenderPass() }; // create the render system with the device and the swap chain render pass

		while (!window.shouldClose())
		{
			glfwPollEvents();
			
			if (auto commandBuffer = renderer.beginFrame()) // begin the frame and get the command buffer
			{
				renderer.beginSwapChainRenderPass(commandBuffer); // begin the render pass for the swap chain
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects); // render the game objects
				renderer.endSwapChainRenderPass(commandBuffer); // end the render pass for the swap chain
				renderer.endFrame(); // end the frame and submit the command buffer
			}
		}

		vkDeviceWaitIdle(device.device()); // wait for the device to finish all operations before destroying resources
	}

	void AppControl::loadGameObjects()
	{
		std::vector<VTAModel::Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		auto model = std::make_shared<VTAModel>(device, vertices);

		auto triangle = VTAGameObject::createGameObject();
		triangle.model = model; // set the model for the game object
		triangle.color = { 0.1f, 0.8f, 0.1f }; // set the color for the game object
		triangle.transform2d.translation.x = 0.2f;
		triangle.transform2d.scale = { 2.f, 0.5f }; 
		triangle.transform2d.rotation = 0.25f * glm::two_pi<float>(); // rotate the triangle by 90 degrees

		gameObjects.push_back(std::move(triangle)); // add the game object to the vector
	}

}




