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


	void PointLightSystem::createDescriptorSetLayouts()
	{
		perObjectLayout = { VTADescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build() };

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
		pipeline = std::make_unique<VTAPipeline>(device, "../shaders/point_light.vert.spv", "../shaders/point_light.frag.spv", pipelineConfig);
	}

	VkPushConstantRange PointLightSystem::createPushConstantRange()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(PointLightPushConstants); // size of the push constant in bytes
		return pushConstantRange;
	}




	PointLightSystem::PointLightSystem(std::string name, VTA::VTADevice& device, ResourceLoader& loader)
		: RenderSystem(name, device, loader)
	{
		
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
		VkDescriptorSet globalSet = frameInfo.globalResourceSet.GetSetForFrame(frameInfo.frameIndex);
		vkCmdBindDescriptorSets
		(frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&globalSet,
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

}
