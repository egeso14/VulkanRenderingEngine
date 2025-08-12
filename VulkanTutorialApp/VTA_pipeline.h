#pragma once
#include <string>
#include <vector>
#include "VTA_device.hpp"

namespace VTA
{
    struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete; // this is to establish unique ownership of resources


        std::vector<VkVertexInputBindingDescription> bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };


    class VTAPipeline
    {
         public:
         VTAPipeline(VTADevice& device, 
                    const std::string& vertFilePath, 
                    const std::string& fragFilePath, 
                    const PipelineConfigInfo& configInfo);
         ~VTAPipeline();
		 VTAPipeline(const VTAPipeline&) = delete;
		 VTAPipeline& operator=(const VTAPipeline&) = delete; // this is to establish unique ownership of resources
		 
		 void bind(VkCommandBuffer commandBuffer);

         static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
         static void enableAlphaBlending(PipelineConfigInfo& configInfo);
           

         private:
         static std::vector<char> readFile(const std::string& filePath);

         void createGraphicsPipeline(const std::string& vertFilePath, 
                                    const std::string& fragFilePath, 
                                    const PipelineConfigInfo& configInfo);

		 void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
         
         VTADevice& VTAdevice;
         VkPipeline graphicsPipeline;
		 VkShaderModule vertShaderModule;
         VkShaderModule fragShaderModule;
    };
}