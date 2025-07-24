#pragma once

#include "VTA_Window.h"
#include "VTA_pipeline.h"
#include "VTA_device.hpp"
#include "VTA_swap_chain.hpp"
#include "VTA_model.h"
#include "VTA_game_object.h"

#include <memory>
#include <vector>

namespace VTA
{
	class AppControl
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;


		AppControl();
		~AppControl();

		AppControl(const AppControl&) = delete;
		AppControl& operator=(const AppControl&) = delete; // this is to establish unique ownership of resources

		void run();
	private:

		void loadGameObjects();
		void createPipelineLayout();
		void createPipeline();
		void freeCommandBuffers();
		void createCommandBuffers();
		void drawFrame();
		void recordCommandBuffer(int imageIndex);
		void recreateSwapChain();
		void renderGameObjects(VkCommandBuffer commandBuffer);


		VTAWindow window{ WIDTH, HEIGHT, "Vulkan Window" };
		VTADevice device{ window };
		std::unique_ptr<VTASwapChain> swapChain;
		std::unique_ptr<VTAPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VTAGameObject> gameObjects;
	};
}