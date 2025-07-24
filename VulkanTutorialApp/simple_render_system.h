#pragma once


#include "VTA_pipeline.h"
#include "VTA_device.hpp"
#include "VTA_model.h"
#include "VTA_game_object.h"

#include <memory>
#include <vector>

namespace VTA
{
	class SimpleRenderSystem
	{
	public:


		SimpleRenderSystem(VTADevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete; // this is to establish unique ownership of resources

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<VTAGameObject>& gameObjects);

	private:

		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);




		VTADevice& device;

		std::unique_ptr<VTAPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}