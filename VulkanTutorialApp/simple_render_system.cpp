#include "simple_render_system.h"
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace VTA
{
	struct SimplePushConstantsData
	{
		glm::mat4 modelMatrix{ 1.f };
		//alignas(16) glm::vec3 color;
		glm::mat4 normalMatrix{ 1.f };
	};


	SimpleRenderSystem::SimpleRenderSystem(VTADevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device{ device }
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass); // create the pipeline with the shader modules and pipeline layout
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}



	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(SimplePushConstantsData); // size of the push constant in bytes


		std::vector<VkDescriptorSetLayout> descriptorSetLayouts { globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()); // Optional
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // info sent to the pipieline other than vertex info
		pipelineLayoutInfo.pushConstantRangeCount = 1; // Very efficient way to send data to shader programs
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}





	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{

		assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline.");


		PipelineConfigInfo pipelineConfig{};
		VTAPipeline::defaultPipelineConfigInfo(pipelineConfig, device.msaaSamples);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<VTAPipeline>(device, "simple_shader.vert.spv", "simple_shader.frag.spv", pipelineConfig);
	}




	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo)
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


		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;

			if (obj.model == nullptr) continue;

			SimplePushConstantsData push{};
			auto modelMatrix = obj.transform.mat4();
			push.modelMatrix = modelMatrix; // use the transform from the game object
			push.normalMatrix = obj.transform.normalMatrix(); // also send the model matrix to the shader

			vkCmdPushConstants(frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantsData),
				&push);
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer); // draw the model with the push constants set
		}

	}




}
