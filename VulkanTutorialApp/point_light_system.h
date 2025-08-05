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
	class PointLightSystem
	{
	public:


		PointLightSystem(VTADevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete; // this is to establish unique ownership of resources

		void render(FrameInfo& frameIndo);

	private:

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);




		VTADevice& device;

		std::unique_ptr<VTAPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}