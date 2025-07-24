#include "AppControl.h"
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


	AppControl::AppControl()
	{
		loadGameObjects(); // load the model data into memory
		createPipelineLayout();
		recreateSwapChain(); 
		createCommandBuffers();
	}

	AppControl::~AppControl()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void AppControl::run()
	{
		while (!window.shouldClose())
		{
			glfwPollEvents();
			drawFrame(); // this will acquire the next image from the swap chain, submit the command buffer, and present the image to the screen
		}

		vkDeviceWaitIdle(device.device()); // wait for the device to finish all operations before destroying resources
	}

	void AppControl::loadGameObjects()
	{
		std::vector<VTAModel::Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		auto model = std::make_shared<VTAModel>(device, vertices);

		auto triangle = VTAGameObject::createGameObject();
		triangle.model = model; // set the model for the game object
		triangle.color = { 0.1f, 0.8f, 0.1f }; // set the color for the game object
		triangle.transform2d.translation.x = 0.2f;
		triangle.transform2d.scale = { 2.f, 0.5f }; 
		triangle.transform2d.rotation = 0.25f * glm::two_pi<float>(); // rotate the triangle by 90 degrees

		gameObjects.push_back(std::move(triangle)); // add the game object to the vector
	}

	void AppControl::createPipelineLayout()
	{

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stages can access the push constant
		pushConstantRange.offset = 0; // offset in bytes from the start of the push constant range
		pushConstantRange.size = sizeof(SimplePushConstantsData); // size of the push constant in bytes

		VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
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

	void AppControl::recreateSwapChain()
	{
		auto extent = window.getExtent();
		while (extent.width == 0 || extent.height == 0) // if the window is minimized, we cannot recreate the swap chain
		{
			extent = window.getExtent();
			glfwWaitEvents(); // wait for an event to occur, such as a window resize
		}

		vkDeviceWaitIdle(device.device()); // wait for the device to finish all operations before recreating the swap chain
		// two swapChains might not be able to exist at the same time
		
		if (swapChain == nullptr)
		{
			swapChain = std::make_unique<VTASwapChain>(device, extent); // recreate the swap chain with the new extent
		}
		else
		{
			swapChain = std::make_unique<VTASwapChain>(device, extent, std::move(swapChain));
			if (swapChain->imageCount() != commandBuffers.size())
			{
				freeCommandBuffers(); // free the old command buffers if the image count has changed
				createCommandBuffers(); // create new command buffers for the new swap chain
			}
		}
		
		createPipeline(); // recreate the pipeline with the new swap chain
		// we cannot get rid of this line even after dynamically creating the scissor and viewport in our pipelineConfig, this is because the renderPass is still dependent on the swapchain
		
		
	}


	void AppControl::freeCommandBuffers()
	{
		vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<float>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear(); // clear the command buffers vector
	}

	void AppControl::createPipeline()
	{
		assert(swapChain != nullptr && "Swap chain must be created before creating the pipeline.");
		assert(pipelineLayout != nullptr && "Pipeline layout must be created before creating the pipeline.");
	

		PipelineConfigInfo pipelineConfig{};
		VTAPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = swapChain->getRenderPass(); // wil change this later when we learn more about render passes
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<VTAPipeline>(device, "simple_shader.vert.spv", "simple_shader.frag.spv", pipelineConfig);
	}
	void AppControl::createCommandBuffers()
	{
		commandBuffers.resize(swapChain->imageCount()); // w or three depending on whether our device supoprts triple buffering
		// this is because we are using a seperate command buffer for each frame buffer
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // primary command buffers can be submitted to a queue, secondary command buffers can be called from primary command buffers but cannot be submitted to a queue
		allocInfo.commandPool = device.getCommandPool(); // command pools are pooling. I think this is a memory optimization
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void AppControl::recordCommandBuffer(int imageIndex)
	{

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->getRenderPass();
		renderPassInfo.framebuffer = swapChain->getFrameBuffer(imageIndex); // which frame buffer is this render pass writing to?
		renderPassInfo.renderArea.offset = { 0, 0 }; // where to start rendering
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent(); // swap chain extent can sometime be larger than the window extent
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		//in our render pass we structured our attachments so that the first attachment is the color attachment and the second attachment is the depth attachment

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // inline means that we are not using secondary command buffers

		// dynamically setup viewport and scissor
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		renderGameObjects(commandBuffers[imageIndex]); // render the game objects to the command buffer
		
		

		// instances can be used when we want to draw multiple copies of the same vertex data
		vkCmdEndRenderPass(commandBuffers[imageIndex]); // end the render pass

		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void AppControl::renderGameObjects(VkCommandBuffer commandBuffer)
	{
		pipeline->bind(commandBuffer); // bind the pipeline to the command buffer

		for (auto& obj : gameObjects)
		{
			obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.001f, glm::two_pi<float>()); // keep the rotation in the range [0, 2pi]

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

	void AppControl::drawFrame()
	{
		uint32_t imageIndex;
		auto result = swapChain->acquireNextImage(&imageIndex); // fetch the index of the frame we should render to next

		if (result == VK_ERROR_OUT_OF_DATE_KHR) // if the swap chain is out of date, we need to recreate it
		{
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) // if we get a suboptimal result, we can still continue rendering
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}


		recordCommandBuffer(imageIndex); // record the command buffer for the current image index
		result = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex); // submit the command buffer to the device graphics queue while handling CPU and GPU synchronization

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) // if the swap chain is out of date or suboptimal, we need to recreate it
		{
			window.resetWindowResizedFlag(); // reset the window resized flag so we don't recreate the swap chain multiple times
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit command buffer!");
		}
	}
}
