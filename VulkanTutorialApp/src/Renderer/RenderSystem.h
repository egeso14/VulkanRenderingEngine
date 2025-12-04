#pragma once
#include "VTA_device.hpp"
#include "VTA_descriptors.h"
#include "Resource.h"

namespace VTA
{
	class RenderSystem
	{
	public:
		RenderSystem(
			std::string name,
			VTA::VTADevice& device,
			ResourceLoader& loader);
		void Initialize(VkRenderPass renderPass);
		virtual std::shared_ptr<VTADescriptorSetLayout> GetPerObjectLayout();
		virtual std::string GetName();

		virtual ~RenderSystem();
	protected:
		virtual VkPushConstantRange createPushConstantRange() = 0;

		virtual void createDescriptorSetLayouts() = 0;
		virtual void createPipelineLayouts(VkPushConstantRange pushConstantRange);
		virtual void createPipeline(VkRenderPass renderPass) = 0;

		VTA::VTADevice& device;
		VkPipelineLayout pipelineLayout;
		std::string name;

		std::shared_ptr<VTA::VTADescriptorSetLayout> perObjectLayout;
		std::shared_ptr<VTA::VTADescriptorSetLayout> globalLayout;
		ResourceLoader& loader;
	};
}