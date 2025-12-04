#pragma once


#include "VTA_pipeline.h"
#include "VTA_device.hpp"
#include "VTA_model.h"
#include "VTA_game_object.h"
#include "VTA_camera.h"
#include "VTA_frame_info.h"
#include "RenderSystem.h"

#include <map>
#include <memory>
#include <vector>

namespace VTA
{
	class PointLightSystem : public RenderSystem
	{
	public:
		PointLightSystem(std::string name,
			VTA::VTADevice& device,
			ResourceLoader& loader);

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete; // this is to establish unique ownership of resources

		void render(FrameInfo& frameIndo);

	private:

		void createDescriptorSetLayouts() override;
		void createPipeline(VkRenderPass renderPass);
		VkPushConstantRange createPushConstantRange() override;

		std::unique_ptr<VTAPipeline> pipeline;
	
	};
}