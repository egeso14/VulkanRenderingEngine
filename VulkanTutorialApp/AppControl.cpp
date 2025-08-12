#include "AppControl.h"
#include "simple_render_system.h"
#include "VTA_camera.h"
#include "keyboard_movement_controller.h"
#include "point_light_system.h"
#include "VTA_Buffer.h"
#include <stdexcept>
#include <array>
#include <chrono>
#include <numeric>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace VTA
{ 


	

	AppControl::AppControl()
	{
		globalPool = VTADescriptorPool::Builder(device)
			.setMaxSets(VTASwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VTASwapChain::MAX_FRAMES_IN_FLIGHT)
			.build(); // create the global descriptor pool

		loadGameObjects(); // load the model data into memory
	}

	AppControl::~AppControl()
	{
		
	}

	void AppControl::run()
	{
		auto minOffsetAllignment = std::lcm(device.properties.limits.minUniformBufferOffsetAlignment, device.properties.limits.nonCoherentAtomSize);

		VTABuffer globalUboBuffer{ device, sizeof(GlobalUbo), VTASwapChain::MAX_FRAMES_IN_FLIGHT,  // how many uniform buffer objects in our uniform buffer? one for each frame in flight
									VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, // double buffering like this should be used whenever we have a synamic resource that is written to each frame
									minOffsetAllignment};

		globalUboBuffer.map(); // map the buffer to host memory so we can write to it

		auto globalSetLayout = VTADescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();


		std::vector<VkDescriptorSet> globalDescriptorSets(VTASwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = globalUboBuffer.descriptorInfo(); // get the descriptor info for the uniform buffer
			VTADescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo) // write the buffer info to the descriptor set
				.build(globalDescriptorSets[i]); // build the descriptor set
		}

		SimpleRenderSystem simpleRenderSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()}; // create the render system with the device and the swap chain render pass
		PointLightSystem pointLightSystemSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() }; // create the render system with the device and the swap chain render pass

        VTACamera camera{};
        //camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
        
        auto viewerObject = VTAGameObject::createGameObject();
		viewerObject.transform.translation = { 0.f, 0.f, -2.5f }; 
        KeyboardMovementController cameraController{};
        
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 5.f));
        auto currentTime = std::chrono::high_resolution_clock::now();



		while (!window.shouldClose())
		{
			glfwPollEvents();
			
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

			if (auto commandBuffer = renderer.beginFrame()) // begin the frame and get the command buffer
			{
				int frameIndex = renderer.getFrameIndex(); // get the current frame index

				FrameInfo frameInfo {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects
				};

				// update
				GlobalUbo ubo{};
				ubo.projectionMatrix = camera.getProjection();
				ubo.viewMatrix = camera.getView();
				pointLightSystemSystem.update(frameInfo, ubo); // update the point light system with the frame info and the uniform buffer object
				globalUboBuffer.writeToIndex(&ubo, frameIndex); // write the projection view matrix to the uniform buffer for the current frame
				globalUboBuffer.flushIndex(frameIndex); // flush the uniform buffer for the current frame

				// render
				renderer.beginSwapChainRenderPass(commandBuffer); // begin the render pass for the swap chain
				simpleRenderSystem.renderGameObjects(frameInfo); // render the game objects
				pointLightSystemSystem.render(frameInfo);
				renderer.endSwapChainRenderPass(commandBuffer); // end the render pass for the swap chain
				renderer.endFrame(); // end the frame and submit the command buffer
			}
		}

		vkDeviceWaitIdle(device.device()); // wait for the device to finish all operations before destroying resources
	}

    

	void AppControl::loadGameObjects()
	{
		std::shared_ptr<VTAModel> cubeModel = VTAModel::createModelFromFile(device, "models/flat_vase.obj"); // load the cube model from the file
		auto cube = VTAGameObject::createGameObject(); // create a game object
		cube.model = cubeModel; // set the model of the game object to the cube model
		cube.transform.translation = { 0.f, 0.f, 0.5f }; // set the translation of the game object
		cube.transform.scale = { 0.5f, 0.5f, 0.5f }; // set the scale of the game object

		std::shared_ptr<VTAModel> quadModel = VTAModel::createModelFromFile(device, "models/quad.obj"); // load the cube model from the file
		auto quad = VTAGameObject::createGameObject(); // create a game object
		quad.model = quadModel; // set the model of the game object to the cube model
		quad.transform.translation = { 0.f, 0.5f, 0.f }; // set the translation of the game object
		quad.transform.scale = { 3.1f, 1.f, 3.f }; // set the scale of the game object

		gameObjects.emplace(cube.getId(), std::move(cube)); // add the game object to the vector of game objects
		gameObjects.emplace(quad.getId(), std::move(quad));
		

		std::vector<glm::vec3> lightColors{
		 {1.f, .1f, .1f},
		 {.1f, .1f, 1.f},
		 {.1f, 1.f, .1f},
		 {1.f, 1.f, .1f},
		 {.1f, 1.f, 1.f},
		 {1.f, 1.f, 1.f}  };

		for (int i = 0; i < lightColors.size(); i++)
		{
			auto pointLight = VTAGameObject::makePointLight(0.2f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight)); // add the point light to the vector of game objects
		}
	}

   

}




