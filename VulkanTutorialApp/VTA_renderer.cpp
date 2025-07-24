#include "VTA_renderer.h"
#include <stdexcept>
#include <array>


namespace VTA
{
	struct SimplePushConstantsData
	{
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};


	VTARenderer::VTARenderer(VTAWindow& window, VTADevice& device) : window{window}, device{device}
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	VTARenderer::~VTARenderer()
	{
		freeCommandBuffers();
	}



	void VTARenderer::recreateSwapChain()
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
			std::shared_ptr<VTASwapChain> oldSwapChain = std::move(swapChain); // move the old swap chain to a shared pointer so we can use it later
			swapChain = std::make_unique<VTASwapChain>(device, extent, std::move(oldSwapChain));

			if (!oldSwapChain->compareSwapFormats(*swapChain.get())) 
			{
				throw std::runtime_error("Swap chain image or depth format has changed!"); // we are no longer recreating the pipeline, so if the render passes are incompatible, then we will return an error
			}


		}
	}


	VkCommandBuffer VTARenderer::beginFrame()
	{
		assert(!isFrameStarted && "Cannot call beginFrame while a frame is already in progress.");

		
		auto result = swapChain->acquireNextImage(&currentImageIndex); // fetch the index of the frame we should render to next

		if (result == VK_ERROR_OUT_OF_DATE_KHR) // if the swap chain is out of date, we need to recreate it
		{
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) // if we get a suboptimal result, we can still continue rendering
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true; // we are now in a frame

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer; // return the command buffer for the current image index
	}


	void VTARenderer::endFrame()
	{
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}


		auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex); // submit the command buffer to the device graphics queue while handling CPU and GPU synchronization

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) // if the swap chain is out of date or suboptimal, we need to recreate it
		{
			window.resetWindowResizedFlag(); // reset the window resized flag so we don't recreate the swap chain multiple times
			recreateSwapChain();
		}

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit command buffer!");
		}

		isFrameStarted = false; // we are no longer in a frame
		currentFrameIndex = (currentFrameIndex + 1) % VTASwapChain::MAX_FRAMES_IN_FLIGHT; 
	}

	void VTARenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Cannot call beginSwapChainRenderPass when frame is not in progress.");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");


		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->getRenderPass();
		renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex); // which frame buffer is this render pass writing to?
		renderPassInfo.renderArea.offset = { 0, 0 }; // where to start rendering
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent(); // swap chain extent can sometime be larger than the window extent
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		//in our render pass we structured our attachments so that the first attachment is the color attachment and the second attachment is the depth attachment

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // inline means that we are not using secondary command buffers

		// dynamically setup viewport and scissor
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VTARenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Cannot call endSwapChainRenderPass when frame is not in progress.");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer); // end the render pass
	}

	void VTARenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<float>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear(); // clear the command buffers vector
	}

	void VTARenderer::createCommandBuffers()
	{
		commandBuffers.resize(VTASwapChain::MAX_FRAMES_IN_FLIGHT); // w or three depending on whether our device supoprts triple buffering
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

}
