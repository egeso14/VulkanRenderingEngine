#include "AppControl.h"
#include "simple_render_system.h"
#include "VTA_camera.h"
#include "keyboard_movement_controller.h"
#include "point_light_system.h"
#include "VTA_Buffer.h"
#include "outline_render_system.h"
#include <stdexcept>
#include <array>
#include <chrono>
#include <numeric>
#include "Resource.h"

#include "UIController.h"
#include "ImGui/ImGuiModule.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>



#define STBI_MSC_SECURE_CRT
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION   // default if not set by build
#include "stb_truetype.h"



namespace VTA
{ 


	

	AppControl::AppControl()
	{

		
		
	}

	AppControl::~AppControl()
	{
		for (int i = 0; i < descriptorAllocators.size(); i++)
		{
			descriptorAllocators[i].clear_pools(device.device());
			descriptorAllocators[i].destroy_pools(device.device());
		}

		
	}

	void AppControl::run()
	{

		ResourceLoader resourceLoader{ VTASwapChain::MAX_FRAMES_IN_FLIGHT, device };


		std::string simple = "SimpleRenderSystem";
		std::string lights = "PointLightRenderSystem";
		std::string editorUI = "EditorUIRenderSystem";
		std::string outline = "OutlineRenderSystem";
		
		SimpleRenderSystem simpleRenderSystem{simple, device, resourceLoader}; // create the render system with the device and the swap chain render pass
		PointLightSystem pointLightSystem{lights, device, resourceLoader}; // create the render system with the device and the swap chain render pass
		OutlineRenderSystem outlineRenderSystem{ outline, device, resourceLoader };

		simpleRenderSystem.Initialize(renderer.getSwapChainRenderPass1());
		pointLightSystem.Initialize(renderer.getSwapChainRenderPass1());
		outlineRenderSystem.Initialize(renderer.getSwapChainRenderPass1());

		
		resourceLoader.InitializeWriters();


		VTA::ImGuiRenderSystem ImGuiRenderSystem;
		ImGuiRenderSystem.OnAttach();
		Game_Module gameModule(resourceLoader, simple, device, VTASwapChain::MAX_FRAMES_IN_FLIGHT);
		VTA_UI::EditorUI_Module editorModule(
			resourceLoader,
			editorUI,
			device,
			window);
		
		
		// need to get all the resource sets from our subsystems and load them
		// also every subsystem needs to get a reference to the resource loader
		// that way they can create their own resource sets even during runtime.


		

		
		/*
		//FontAtlas fontAtlas;
		//BuildFontAtlas("fonts\Inter_default.ttf", 32, 32, 95, 512, 512,fontAtlas);
		VTA_Image::Texture fontAtlasTexture(device, *fontAtlas);
		
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
			auto imageInfo = testTexture.descriptorInfo();
			auto fontImageInfo = fontAtlasTexture.descriptorInfo();

			VkDescriptorSet globalDescriptorSet = descriptorAllocators[i].allocate( // allocate a descriptor set from the descriptor allocator
				device.device(),
				globalSetLayout->getDescriptorSetLayout()
			);

			VkDescriptorSet editorUIDescriptorSet = descriptorAllocators[i].allocate(
				device.device(),
				uiSetLayout->getDescriptorSetLayout()
			);

			VTADescriptorWriter writer{ *globalSetLayout };
			writer.writeBuffer(0, &bufferInfo); // write to the writes vector
			writer.writeImage(1, &imageInfo);
			writer.overwrite(globalDescriptorSet, device); // bind descriptor sets to the buffers in the writes vector

			VTADescriptorWriter fontWriter { *uiSetLayout };
			fontWriter.writeImage(0, &fontImageInfo);
			fontWriter.overwrite(editorUIDescriptorSet, device);

			globalDescriptorSets.push_back(globalDescriptorSet);
			editorUIDescriptorSets.push_back(editorUIDescriptorSet);
		}*/

		

		

		


        auto currentTime = std::chrono::high_resolution_clock::now();

		

		while (!window.shouldClose())
		{
			glfwPollEvents();
			
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);
            float aspect = renderer.getAspectRatio();
            
			

			renderer.beginMainFrame(); // begin the frame and get the command buffer
			
			int frameIndex = renderer.getFrameIndex(); // get the current frame index

			glm::vec2 screenDims = { renderer.getScreenWidth(), renderer.getScreenHeight() };
			
			

			// render game
			auto commandBuffer = renderer.beginGameCommandBuffer();
			auto frameInfo = gameModule.Update(window.getGLFWwindow(), frameTime, aspect, frameIndex, screenDims, commandBuffer);
			renderer.beginSwapChainRenderPass1(commandBuffer); // begin the render pass for the swap chain
			simpleRenderSystem.renderGameObjects(frameInfo); // render the game objects
			pointLightSystem.render(frameInfo);
			outlineRenderSystem.renderGameObjects(frameInfo);
			renderer.endSwapChainRenderPass(commandBuffer); // end the render pass for the swap chain
			renderer.endGameCommandBuffer();
			// render editor ui
			ImGuiRenderSystem.BeginUIFrame();
			editorModule.Update();
			ImGuiRenderSystem.EndUIFrame();
			renderer.endMainFrame(); // end the frame and submit the command buffer
				
			
		}

		vkDeviceWaitIdle(device.device()); // wait for the device to finish all operations before destroying resources
		ImGuiRenderSystem.OnDetach();
	}

   

}




