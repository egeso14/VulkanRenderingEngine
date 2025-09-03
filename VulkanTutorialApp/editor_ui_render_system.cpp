#include "editor_ui_render_system.h"
#include <stdexcept>
#include <deque>

namespace VTA_UI
{
	struct UIPushConstantsData
	{
		glm::mat4 modelMatrix{ 1.f };
		alignas(16) glm::vec3 color;
	};

	EditorUIRenderSystem::EditorUIRenderSystem(VTA::VTADevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device {device}
	{
		createPipelineLayouts(globalSetLayout);
		createPipelines(renderPass); // create the pipeline with the shader modules and pipeline layout
	}

	EditorUIRenderSystem::~EditorUIRenderSystem()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}



	void EditorUIRenderSystem::createPipelineLayouts(VkDescriptorSetLayout uiDescriptorSetLayout)
	{

		// create pipeline layout for rendering text
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(UIPushConstantsData); // size of the push constant in bytes


		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ uiDescriptorSetLayout };

		VkPipelineLayoutCreateInfo textPipelineLayoutInfo{};
		textPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		textPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()); // Optional
		textPipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // info sent to the pipieline other than vertex info
		textPipelineLayoutInfo.pushConstantRangeCount = 1; // Very efficient way to send data to shader programs
		textPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device.device(), &textPipelineLayoutInfo, nullptr, &textPipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create text pipeline layout!");
		}

		// create pipeline layout for rendering simple shapes
		
		VkPipelineLayoutCreateInfo shapePipelineLayoutInfo{};
		shapePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		shapePipelineLayoutInfo.setLayoutCount = 0; // Optional
		shapePipelineLayoutInfo.pSetLayouts = nullptr; // info sent to the pipieline other than vertex info
		shapePipelineLayoutInfo.pushConstantRangeCount = 1; // Very efficient way to send data to shader programs
		shapePipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device.device(), &shapePipelineLayoutInfo, nullptr, &shapePipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shape pipeline layout!");
		}

	}





	void EditorUIRenderSystem::createPipelines(VkRenderPass renderPass)
	{

		assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline.");


		VTA::PipelineConfigInfo pipelineConfig{};
		VTA::VTAPipeline::defaultPipelineConfigInfo(pipelineConfig, device.msaaSamples);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<VTA::VTAPipeline>(device, "simple_shader.vert.spv", "simple_shader.frag.spv", pipelineConfig);
	}




	void EditorUIRenderSystem::renderWidgets(VTA::FrameInfo& frameInfo)
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


		for (auto& kv : frameInfo.topLevelWidgets)
		{
			UiGraphNode topLevelNode = kv->createUiGraph();
			std::deque<UiGraphNode> next(topLevelNode.children.begin(), topLevelNode.children.end());

			while (!next.empty())
			{
				UiGraphNode current = next.front();
				next.pop_front();

				if (current.isText)
				{

				}
			}


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