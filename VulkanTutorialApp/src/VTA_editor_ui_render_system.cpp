#include "VTA_editor_ui_render_system.h"
#include <stdexcept>
#include <deque>
#include <cassert>

namespace VTA_UI
{
	struct UIPushConstantsData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 projectionMatrix{ 1.f };
		alignas(16) glm::vec4 color;
	};

	EditorUIRenderSystem::EditorUIRenderSystem(VTA::VTADevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device {device}
	{
		createPipelineLayouts(globalSetLayout);
		createPipelines(renderPass); // create the pipeline with the shader modules and pipeline layout
	}

	EditorUIRenderSystem::~EditorUIRenderSystem()
	{
		vkDestroyPipelineLayout(device.device(), shapePipelineLayout, nullptr);
		vkDestroyPipelineLayout(device.device(), textPipelineLayout, nullptr);
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





	void EditorUIRenderSystem::createPipelines(VkRenderPass uiRenderPass)
	{

		assert(shapePipelineLayout != nullptr && textPipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline.");

		/*
		VTA::PipelineConfigInfo shapePipelineConfig{};
		VTA::VTAPipeline::defaultPipelineConfigInfo(shapePipelineConfig, VK_SAMPLE_COUNT_1_BIT);
		shapePipelineConfig.renderPass = uiRenderPass;
		shapePipelineConfig.pipelineLayout = shapePipelineLayout;
		shapePipeline = std::make_unique<VTA::VTAPipeline>(device, "simple_shader.vert.spv", "simple_shader.frag.spv", shapePipelineConfig);*/

		VTA::PipelineConfigInfo textPipelineConfig{};
		VTA::VTAPipeline::defaultPipelineConfigInfo(textPipelineConfig, VK_SAMPLE_COUNT_1_BIT);
		textPipelineConfig.renderPass = uiRenderPass;
		textPipelineConfig.pipelineLayout = textPipelineLayout;
		textPipeline = std::make_unique<VTA::VTAPipeline>(device, "text_shader.vert.spv", "text_shader.frag.spv", textPipelineConfig);
	}

	void EditorUIRenderSystem::renderWidgets(FrameInfo_EditorUI frameInfo)
	{
		// let's experiment with binding pipelines dynamically depending on the type of the widget object to see the cost of doing so
		
		auto& allWidgets = VTAWidget::getWidgetMap();
		auto& topLevelWidgets = VTAWidget::getTopLevelWidgets();

		for (auto& topLevelWidget : topLevelWidgets)
		{
			VTAWidget::UiGraphNode topLevelNode = topLevelWidget->CreateUiGraph(glm::mat4(1.0f));
			std::deque<VTAWidget::UiGraphNode> next(topLevelNode.children.begin(), topLevelNode.children.end());

			while (!next.empty())
			{
				VTAWidget::UiGraphNode current = next.front();
				next.pop_front();

				if (current.isText)
				{
					textPipeline->bind(frameInfo.commandBuffer); // bind the pipeline to the command buffer

					// if we are rendering text we need to bind the descriptor sets that have the font atlas
					// does this get overwritten when we bind a pipeline that doesn't use descriptor sets/
					vkCmdBindDescriptorSets
					(frameInfo.commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						textPipelineLayout,
						0, 1,
						&frameInfo.uiDescriptorSet,
						0,
						nullptr);
				}

				else
				{

					shapePipeline->bind(frameInfo.commandBuffer);
				}

				// bind the model and draw

				auto pair = allWidgets.find(topLevelNode.widgetId);

				pair->second->model->bind(frameInfo.commandBuffer);
				pair->second->model->draw(frameInfo.commandBuffer);

				for (auto child : current.children)
				{
					next.push_front(child);
				}
			}
		}

	}
}