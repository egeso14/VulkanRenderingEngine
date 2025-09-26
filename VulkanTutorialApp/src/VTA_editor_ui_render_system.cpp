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

		
		VTA::PipelineConfigInfo shapePipelineConfig{};
		VTA::VTAPipeline::defaultPipelineConfigInfo(shapePipelineConfig, VK_SAMPLE_COUNT_1_BIT);
		shapePipelineConfig.bindingDescription = FlatMesh::Vertex::getBindingDescriptions();
		shapePipelineConfig.attributeDescriptions = FlatMesh::Vertex::getAttributeDescriptions();
		shapePipelineConfig.renderPass = uiRenderPass;
		shapePipelineConfig.pipelineLayout = shapePipelineLayout;
		shapePipeline = std::make_unique<VTA::VTAPipeline>(device, "../shaders/shape_shader.vert.spv", "../shaders/shape_shader.frag.spv", shapePipelineConfig);

		VTA::PipelineConfigInfo textPipelineConfig{};
		VTA::VTAPipeline::defaultPipelineConfigInfo(textPipelineConfig, VK_SAMPLE_COUNT_1_BIT);
		// set color blend info here because we want alpha blending anti-aliasing
		textPipelineConfig.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		textPipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
		textPipelineConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;   // Optional
		textPipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // Optional
		textPipelineConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		textPipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		textPipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // Optional
		textPipelineConfig.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		textPipelineConfig.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		textPipelineConfig.colorBlendInfo.logicOpEnable = VK_FALSE;
		textPipelineConfig.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		textPipelineConfig.colorBlendInfo.attachmentCount = 1;

		textPipelineConfig.colorBlendInfo.pAttachments = &textPipelineConfig.colorBlendAttachment;
		textPipelineConfig.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		textPipelineConfig.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		textPipelineConfig.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		textPipelineConfig.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		
		textPipelineConfig.bindingDescription = FlatMesh::Vertex::getBindingDescriptions();
		textPipelineConfig.attributeDescriptions = FlatMesh::Vertex::getAttributeDescriptions();
		textPipelineConfig.renderPass = uiRenderPass;
		textPipelineConfig.pipelineLayout = textPipelineLayout;
		textPipeline = std::make_unique<VTA::VTAPipeline>(device, "../shaders/text_shader.vert.spv", "../shaders/text_shader.frag.spv", textPipelineConfig);
	}

	void EditorUIRenderSystem::renderWidgets(FrameInfo_EditorUI frameInfo)
	{
		// let's experiment with binding pipelines dynamically depending on the type of the widget object to see the cost of doing so
		
		auto& topLevelWidgets = VTAWidget::getTopLevelWidgets();
		
		
		for (auto& topLevelWidget : topLevelWidgets)
		{
			
			VTAWidget::UiGraphNode topLevelNode = topLevelWidget->createUiGraph(glm::vec2{ 0, 0 }, glm::vec2{ frameInfo.screenWidth, frameInfo.screenHeight });
			std::deque<VTAWidget::UiGraphNode> next;
			next.push_front(topLevelNode);

			
			// before we bind our model we need to possibly make adjustments to it due to the model being saved in pixel values 
			// and pixel values are unstable when we want a widget to scale with the screen


			while (!next.empty())
			{
				VTAWidget::UiGraphNode current = next.front();
				next.pop_front();
				VkPipelineLayout correctPipelineLayout;

				if (current.isText)
				{
					textPipeline->bind(frameInfo.commandBuffer); // bind the pipeline to the command buffer

					vkCmdBindDescriptorSets
					(frameInfo.commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						textPipelineLayout,
						0, 1,
						&frameInfo.uiDescriptorSet,
						0,
						nullptr);
					correctPipelineLayout = textPipelineLayout;
				}

				else
				{
					shapePipeline->bind(frameInfo.commandBuffer);
					correctPipelineLayout = shapePipelineLayout;
				}

				// bind the model and draw
				UIPushConstantsData push{};
				push.modelMatrix = current.modelMatrix;
				push.projectionMatrix = current.widget->rectTransform.getProjection(frameInfo.screenWidth, frameInfo.screenHeight);

				vkCmdPushConstants(frameInfo.commandBuffer,
					correctPipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(UIPushConstantsData),
					&push);


				current.widget->model->bind(frameInfo.commandBuffer);
				current.widget->model->draw(frameInfo.commandBuffer);

				for (auto child : current.children)
				{
					next.push_front(child);
				}
			}
		}

	}
}