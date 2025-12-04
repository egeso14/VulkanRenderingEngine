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
	class SimpleRenderSystem : public RenderSystem
	{
	public:

		SimpleRenderSystem(std::string name,
			VTA::VTADevice& device,
			ResourceLoader& loader);
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete; // this is to establish unique ownership of resources
		void renderGameObjects(FrameInfo &frameIndo);
		static std::unique_ptr<ResourceSet> CreatePerObjectResourceSet();
		static std::unique_ptr<ResourceSet> CreateGlobalResourceSet();

	private:
		void createDescriptorSetLayouts() override;
		void createPipeline(VkRenderPass renderPass) override;
		VkPushConstantRange createPushConstantRange() override;

		
		std::unique_ptr<VTAPipeline> pipeline;
	};
}