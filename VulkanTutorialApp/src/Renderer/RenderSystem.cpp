#include <stdexcept>
#include "RenderSystem.h"

VTA::RenderSystem::RenderSystem(
	std::string name,
	VTA::VTADevice& device,
	ResourceLoader& loader) :
	device(device), loader(loader)
	
{
	this->name = name;
}

void VTA::RenderSystem::Initialize(VkRenderPass renderPass)
{
	createDescriptorSetLayouts();
	auto pushConstantRange = createPushConstantRange();
	createPipelineLayouts(pushConstantRange);
	createPipeline(renderPass);
}

VTA::RenderSystem::~RenderSystem()
{
	vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

std::shared_ptr<VTA::VTADescriptorSetLayout> VTA::RenderSystem::GetPerObjectLayout()
{
	return perObjectLayout;
}

std::string VTA::RenderSystem::GetName()
{
	return name;
}

void VTA::RenderSystem::createPipelineLayouts(VkPushConstantRange pushConstantRange)
{

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{  };
	
	if (globalLayout)
	{
		descriptorSetLayouts.push_back(globalLayout->getDescriptorSetLayout());
	}

	if (perObjectLayout)
	{
		descriptorSetLayouts.push_back(perObjectLayout->getDescriptorSetLayout());
	}



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






