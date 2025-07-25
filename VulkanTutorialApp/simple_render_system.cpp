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
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};


	SimpleRenderSystem::SimpleRenderSystem(VTADevice& device, VkRenderPass renderPass) : device{ device }
	{
		createPipelineLayout();
		createPipeline(renderPass); // create the pipeline with the shader modules and pipeline layout
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}



	void SimpleRenderSystem::createPipelineLayout()
	{

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(SimplePushConstantsData); // size of the push constant in bytes

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // info sent to the pipieline other than vertex info
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
		VTAPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<VTAPipeline>(device, "simple_shader.vert.spv", "simple_shader.frag.spv", pipelineConfig);
	}




	void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<VTAGameObject>& gameObjects)
	{
		pipeline->bind(commandBuffer); // bind the pipeline to the command buffer

		for (auto& obj : gameObjects)
		{
			SimplePushConstantsData push{};
			push.offset = obj.transform2d.translation; // use the translation from the game object transform
			push.color = obj.color; // use the color from the game object
			push.transform = obj.transform2d.mat2(); // use the transform from the game object

			vkCmdPushConstants(commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantsData),
				&push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer); // draw the model with the push constants set
		}

	}




}
