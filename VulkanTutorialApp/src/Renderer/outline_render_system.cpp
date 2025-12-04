#include "outline_render_system.h"
#include <stdexcept>
#include <array>
#include "VTA_swap_chain.hpp"
#include <memory>
#include "VTA_descriptors.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace VTA
{
	struct OutlinePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
		//alignas(16) glm::vec3 color;
		glm::mat4 normalMatrix{ 1.f };
	};




	void OutlineRenderSystem::createDescriptorSetLayouts()
	{
        	globalLayout = VTADescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();
	}


	void OutlineRenderSystem::createPipeline(VkRenderPass renderPass)
	{

		assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline.");


		PipelineConfigInfo pipelineConfig{};
		VTAPipeline::defaultPipelineConfigInfo(pipelineConfig, device.msaaSamples);

		

		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};

		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE; // use this if you only want to use the first few stages of the pipeline
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.lineWidth = 1.0f;
		rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT; // this relates to the winding order.
		rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional 
		pipelineConfig.rasterizationInfo = rasterizationInfo;

		pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
		pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		pipelineConfig.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		pipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
		pipelineConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;   // Optional
		pipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // Optional
		pipelineConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		pipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // Optional
		pipelineConfig.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		pipelineConfig.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineConfig.colorBlendInfo.logicOpEnable = VK_FALSE;
		pipelineConfig.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		pipelineConfig.colorBlendInfo.attachmentCount = 1;

		pipelineConfig.colorBlendInfo.pAttachments = &pipelineConfig.colorBlendAttachment;
		pipelineConfig.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		pipelineConfig.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		pipelineConfig.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		pipelineConfig.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		//pipelineConfig.rasterizationInfo = rasterizationInfo;

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<VTAPipeline>(device, "../shaders/model_outline.vert.spv", "../shaders/model_outline.frag.spv", pipelineConfig);
	}

	VkPushConstantRange OutlineRenderSystem::createPushConstantRange()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(OutlinePushConstantData); // size of the push constant in bytes
		return pushConstantRange;
	}


	OutlineRenderSystem::OutlineRenderSystem(std::string name, VTA::VTADevice& device, ResourceLoader& loader)
		: RenderSystem(name, device, loader)
	{
	}

	void OutlineRenderSystem::renderGameObjects(FrameInfo& frameInfo)
	{
		pipeline->bind(frameInfo.commandBuffer); // bind the pipeline to the command buffer
		VkDescriptorSet globalSet = frameInfo.globalResourceSet.GetSetForFrame(frameInfo.frameIndex);

		vkCmdBindDescriptorSets
		(frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&globalSet,
			0,
			nullptr);


		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;

			if (obj.model == nullptr || obj.outlineComponent == nullptr) continue;

			OutlinePushConstantData push{};
			auto modelMatrix = obj.transform.mat4();
			push.modelMatrix = modelMatrix; // use the transform from the game object
			push.normalMatrix = obj.transform.normalMatrix(); // also send the model matrix to the shader


			vkCmdPushConstants(frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(OutlinePushConstantData),
				&push);

			
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer); // draw the model with the push constants set
		}

	}

}
