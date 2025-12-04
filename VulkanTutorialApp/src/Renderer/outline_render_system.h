#pragma once


#include "VTA_pipeline.h"
#include "VTA_device.hpp"
#include "VTA_model.h"
#include "VTA_game_object.h"
#include "VTA_camera.h"
#include "VTA_frame_info.h"
#include "Resource.h"
#include "RenderSystem.h"


#include <memory>
#include <vector>

namespace VTA
{
	class OutlineRenderSystem : public RenderSystem
	{
	public:

		OutlineRenderSystem(std::string name,
			VTA::VTADevice& device,
			ResourceLoader& loader);
		OutlineRenderSystem(const OutlineRenderSystem&) = delete;
		OutlineRenderSystem& operator=(const OutlineRenderSystem&) = delete; // this is to establish unique ownership of resources
		void renderGameObjects(FrameInfo &frameIndo);

	private:
		void createDescriptorSetLayouts() override;
		void createPipeline(VkRenderPass renderPass) override;
		VkPushConstantRange createPushConstantRange() override;

		
		std::unique_ptr<VTAPipeline> pipeline;
	};
}