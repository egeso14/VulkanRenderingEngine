#include "simple_render_system.h"
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
	struct SimplePushConstantsData
	{
		glm::mat4 modelMatrix{ 1.f };
		//alignas(16) glm::vec3 color;
		glm::mat4 normalMatrix{ 1.f };
	};




	void SimpleRenderSystem::createDescriptorSetLayouts()
	{
		globalLayout = VTADescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();
		
		perObjectLayout = VTADescriptorSetLayout::Builder(device)
		.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

		loader.RegisterLayout("GlobalLayout", globalLayout);
		loader.RegisterLayout("SimplePerObjectLayout", perObjectLayout);
	}


	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{

		assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline.");


		PipelineConfigInfo pipelineConfig{};
		VTAPipeline::defaultPipelineConfigInfo(pipelineConfig, device.msaaSamples);




		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<VTAPipeline>(device, "../shaders/simple_shader.vert.spv", "../shaders/simple_shader.frag.spv", pipelineConfig);
	}

	VkPushConstantRange SimpleRenderSystem::createPushConstantRange()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(SimplePushConstantsData); // size of the push constant in bytes
		return pushConstantRange;
	}


	SimpleRenderSystem::SimpleRenderSystem(std::string name, VTA::VTADevice& device, ResourceLoader& loader)
		: RenderSystem(name, device, loader)
	{
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo)
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

			auto objSet = obj.resources->GetSetForFrame(frameInfo.frameIndex);

			vkCmdBindDescriptorSets
			(frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				1, 1,
				&objSet,
				0,
				nullptr);

			
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer); // draw the model with the push constants set
		}

	}

	std::unique_ptr<VTA::ResourceSet> SimpleRenderSystem::CreatePerObjectResourceSet()
	{
		auto frames = VTASwapChain::MAX_FRAMES_IN_FLIGHT;
		return std::move(std::make_unique<VTA::ResourceSet>("SimplePerObjectLayout", frames));
	}
	std::unique_ptr<ResourceSet> VTA::SimpleRenderSystem::CreateGlobalResourceSet()
	{
		auto frames = VTASwapChain::MAX_FRAMES_IN_FLIGHT;
		return std::move(std::make_unique<ResourceSet>("GlobalLayout", frames));
	}

}
