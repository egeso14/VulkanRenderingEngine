#pragma once


#include "VTA_pipeline.h"
#include "VTA_device.hpp"
#include "VTA_model.h"
#include "VTA_game_object.h"
#include "VTA_camera.h"
#include "VTA_frame_info.h"

#include <memory>
#include <vector>

namespace VTA
{
	class SimpleRenderSystem
	{
	public:


		SimpleRenderSystem(VTADevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete; // this is to establish unique ownership of resources

		void renderGameObjects(FrameInfo &frameIndo);

	private:

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
		void createPipeline(VkRenderPass renderPass);




		VTADevice& device;

		std::unique_ptr<VTAPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}