#include "point_light_system.h"
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace VTA
{


	PointLightSystem::PointLightSystem(VTADevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device{ device }
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass); // create the pipeline with the shader modules and pipeline layout
	}

	PointLightSystem::~PointLightSystem()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}



	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{

		/*VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(SimplePushConstantsData); // size of the push constant in bytes */


		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()); // Optional
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // info sent to the pipieline other than vertex info
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Very efficient way to send data to shader programs
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}





	void PointLightSystem::createPipeline(VkRenderPass renderPass)
	{

		assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline.");


		PipelineConfigInfo pipelineConfig{};
		VTAPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.bindingDescription.clear();
		pipelineConfig.attributeDescriptions.clear();

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<VTAPipeline>(device, "point_light.vert.spv", "point_light.frag.spv", pipelineConfig);
	}




	void PointLightSystem::render(FrameInfo& frameInfo)
	{
		pipeline->bind(frameInfo.commandBuffer); // bind the pipeline to the command buffer

		vkCmdBindDescriptorSets
		(frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);


		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);

	}
}
