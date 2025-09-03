#include "point_light_system.h"
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace VTA
{

	struct PointLightPushConstants
	{
		glm::vec4 position;
		glm::vec4 color{};
		float radius;

	};


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

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(PointLightPushConstants); // size of the push constant in bytes 


		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()); // Optional
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // info sent to the pipieline other than vertex info
		pipelineLayoutInfo.pushConstantRangeCount = 1; // Very efficient way to send data to shader programs
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			
		}
	}





	void PointLightSystem::createPipeline(VkRenderPass renderPass)
	{

		assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline.");


		PipelineConfigInfo pipelineConfig{};
		VTAPipeline::defaultPipelineConfigInfo(pipelineConfig, device.msaaSamples);
		VTAPipeline::enableAlphaBlending(pipelineConfig); // enable alpha blending for the point light system
		pipelineConfig.bindingDescription.clear();
		pipelineConfig.attributeDescriptions.clear();

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<VTAPipeline>(device, "point_light.vert.spv", "point_light.frag.spv", pipelineConfig);
	}




	void PointLightSystem::render(FrameInfo& frameInfo)
	{
		// sort lights
		std::map<float, VTAGameObject::id_t> sorted;
		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;
			auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
			sorted[glm::dot(offset, offset)] = obj.getId();
		}




		pipeline->bind(frameInfo.commandBuffer); // bind the pipeline to the command buffer

		vkCmdBindDescriptorSets
		(frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
		{
			auto& obj = frameInfo.gameObjects.at(it->second);
			if (obj.pointLight == nullptr) continue;

			PointLightPushConstants push{};
			push.position = glm::vec4(obj.transform.translation, 1.f);
			push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			push.radius = obj.transform.scale.x;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(PointLightPushConstants), &push); // push the constants to the shader
			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0); // draw the point light as a quad
		}

		

	}
	void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
	{

		int lightIndex = 0;
		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			ubo.pointLightS[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
			ubo.pointLightS[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			lightIndex++;
		}

		ubo.numLights = lightIndex;
	}
}
