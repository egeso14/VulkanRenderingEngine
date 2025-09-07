
#include "AppControl.h"
#include "simple_render_system.h"
#include "VTA_camera.h"
#include "keyboard_movement_controller.h"
#include "point_light_system.h"
#include "VTA_Buffer.h"
#include "VTA_image.h"
#include <stdexcept>
#include <array>
#include <chrono>
#include <numeric>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Tracy.hpp"
#include <TracyVulkan.hpp>


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
		ZoneScoped;
		

		descriptorAllocators = std::vector<VTADescriptorAllocatorGrowable>(VTASwapChain::MAX_FRAMES_IN_FLIGHT);


		auto minOffsetAllignment = std::lcm(device.properties.limits.minUniformBufferOffsetAlignment, device.properties.limits.nonCoherentAtomSize);

		VTABuffer globalUboBuffer{ device, sizeof(GlobalUbo), VTASwapChain::MAX_FRAMES_IN_FLIGHT,  // how many uniform buffer objects in our uniform buffer? one for each frame in flight
									VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, // double buffering like this should be used whenever we have a synamic resource that is written to each frame
									minOffsetAllignment};

		globalUboBuffer.map(); // map the buffer to host memory so we can write to it

		auto globalSetLayout = VTADescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		auto textureSetLayout = VTADescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		// setting up tracy for Vulkan
		auto cmd = device.beginSingleTimeCommands();
		auto tracyVkCtx = TracyVkContext(device.physicalDevice, device.device(), device.graphicsQueue(), cmd);
		device.endSingleTimeCommands(cmd);
		
		for (int i = 0; i < descriptorAllocators.size(); i++)
		{
			std::vector < VTADescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
			};


			descriptorAllocators[i] = VTADescriptorAllocatorGrowable{};
			descriptorAllocators[i].init(device.device(), 1000, frame_sizes);
			
			auto bufferInfo = globalUboBuffer.descriptorInfo(); // get the descriptor info for the uniform buffer
			auto testTextureInfo = testTexture.descriptorInfo();
			auto mipmapTextureInfo = mipmapTexture.descriptorInfo();


			VkDescriptorSet globalDescriptorSet = descriptorAllocators[i].allocate( // allocate a descriptor set from the descriptor allocator
				device.device(),
				globalSetLayout->getDescriptorSetLayout()
			);

			VkDescriptorSet testTextureDescriptorSet = descriptorAllocators[i].allocate(
				device.device(),
				textureSetLayout->getDescriptorSetLayout()
			);

			VkDescriptorSet mipmapTextureDescriptorSet = descriptorAllocators[i].allocate(
				device.device(),
				textureSetLayout->getDescriptorSetLayout()
			);

			VTADescriptorWriter globalSetWriter{ *globalSetLayout };
			globalSetWriter.writeBuffer(0, &bufferInfo); // write to the writes vector
			globalSetWriter.overwrite(globalDescriptorSet, device); // bind descriptor sets to the buffers in the writes vector

			VTADescriptorWriter testTextureSetWriter{ *textureSetLayout };
			testTextureSetWriter.writeImage(0, &testTextureInfo);
			testTextureSetWriter.overwrite(testTextureDescriptorSet, device);

			VTADescriptorWriter mipmapTextureSetWriter{ *textureSetLayout };
			mipmapTextureSetWriter.writeImage(0, &mipmapTextureInfo);
			mipmapTextureSetWriter.overwrite(mipmapTextureDescriptorSet, device);



			descriptorSets.push_back({ globalDescriptorSet, testTextureDescriptorSet, mipmapTextureDescriptorSet });
		}

		SimpleRenderSystem simpleRenderSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()}; // create the render system with the device and the swap chain render pass
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
				TracyVkZone(tracyVkCtx, commandBuffer, "GBuffer");
				int frameIndex = renderer.getFrameIndex(); // get the current frame index
				
				FrameInfo frameInfo {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					descriptorSets[frameIndex],
					gameObjects
				};

				// update
				GlobalUbo ubo{};
				ubo.projectionMatrix = camera.getProjection();
				ubo.viewMatrix = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystemSystem.update(frameInfo, ubo); // update the point light system with the frame info and the uniform buffer object
				globalUboBuffer.writeToIndex(&ubo, frameIndex); // write the projection view matrix to the uniform buffer for the current frame
				globalUboBuffer.flushIndex(frameIndex); // flush the uniform buffer for the current frame
				
				// render
				
				renderer.beginSwapChainRenderPass(commandBuffer); // begin the render pass for the swap chain
				simpleRenderSystem.renderGameObjects(frameInfo); // render the game objects
				pointLightSystemSystem.render(frameInfo);
				
				FrameMark;
				TracyVkCollect(tracyVkCtx, commandBuffer);
				
				renderer.endSwapChainRenderPass(commandBuffer); // end the render pass for the swap chain
				renderer.endFrame(); // end the frame and submit the command buffer
				
			}
		}
		TracyVkDestroy(tracyVkCtx);
		vkDeviceWaitIdle(device.device()); // wait for the device to finish all operations before destroying resources
	}

    

	void AppControl::loadGameObjects()
	{
		std::shared_ptr<VTAModel> vaseModel1 = VTAModel::createModelFromFile(device, "models/smooth_vase.obj"); // load the cube model from the file
		auto vase1 = VTAGameObject::createGameObject(); // create a game object
		vase1.model = vaseModel1; // set the model of the game object to the cube model
		vase1.transform.translation = { 0.f, 0.4f, 0.5f }; // set the translation of the game object
		vase1.transform.scale = { 1.f, 1.f, 1.f }; // set the scale of the game object
		//cube.transform.rotation = { glm::radians(180.f), 0.f , 0.f };
		vase1.model->textureDSindex = 1;

		std::shared_ptr<VTAModel> vaseModel2 = VTAModel::createModelFromFile(device, "models/smooth_vase.obj"); // load the cube model from the file
		auto vase2 = VTAGameObject::createGameObject(); // create a game object
		vase2.model = vaseModel2; // set the model of the game object to the cube model
		vase2.transform.translation = { 0.f, 0.4f, -0.5f }; // set the translation of the game object
		vase2.transform.scale = { 1.f, 1.f, 1.f }; // set the scale of the game object
		//cube.transform.rotation = { glm::radians(180.f), 0.f , 0.f };
		vase2.model->textureDSindex = 1;





		gameObjects.emplace(vase2.getId(), std::move(vase2)); // add the game object to the vector of game objects
		gameObjects.emplace(vase1.getId(), std::move(vase1)); // add the game object to the vector of game objects

				std::shared_ptr<VTAModel> quadModel = VTAModel::createModelFromFile(device, "models/quad.obj"); // load the cube model from the file
				auto quad = VTAGameObject::createGameObject(); // create a game object
				quad.model = quadModel; // set the model of the game object to the cube model
				quad.transform.translation = { 0, 0.5f, 0 }; // set the translation of the game object
				quad.transform.scale = { 3.f, 3.f, 3.f }; // set the scale of the game object
				quad.model->textureDSindex = 1;
				gameObjects.emplace(quad.getId(), std::move(quad));

				
						auto pointLight = VTAGameObject::makePointLight(1.f);
						pointLight.color = { 1.f, 1.f, 1.f };

						pointLight.transform.translation = glm::vec3(0, -0.5f, 0);
						gameObjects.emplace(pointLight.getId(), std::move(pointLight)); // add the point light to the vector of game objects
				
		
		
		

		
	
		
	}



   

}




