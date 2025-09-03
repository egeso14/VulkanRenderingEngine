#pragma once

#include "VTA_pipeline.h"
#include "VTA_device.hpp"
#include "VTA_model.h"
#include "VTA_game_object.h"
#include "VTA_camera.h"
#include "VTA_frame_info.h"


#include <memory>
#include <vector>

namespace VTA_UI
{
	class EditorUIRenderSystem
	{
	public:


		EditorUIRenderSystem(VTA::VTADevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~EditorUIRenderSystem();

		EditorUIRenderSystem(const EditorUIRenderSystem&) = delete;
		EditorUIRenderSystem& operator=(const EditorUIRenderSystem&) = delete; // this is to establish unique ownership of resources

		void renderWidgets(VTA::FrameInfo& frameIndo);

	private:

		void createPipelineLayouts(VkDescriptorSetLayout globalSetLayout);
		void createPipelines(VkRenderPass renderPass);




		VTA::VTADevice& device;

		std::unique_ptr<VTA::VTAPipeline> textPipeline;
		VkPipelineLayout textPipelineLayout;

		std::unique_ptr<VTA::VTAPipeline> shapePipeline;
		VkPipelineLayout shapePipelineLayout
	};
}