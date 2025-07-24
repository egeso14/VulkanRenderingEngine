#pragma once

#include "VTA_Window.h"
#include "VTA_device.hpp"
#include "VTA_swap_chain.hpp"
#include "VTA_model.h"

#include <memory>
#include <vector>
#include <cassert>

namespace VTA
{
	class VTARenderer
	{
	public:

		VTARenderer(VTAWindow& window, VTADevice& device);
		~VTARenderer();

		VTARenderer(const VTARenderer&) = delete;
		VTARenderer& operator=(const VTARenderer&) = delete; // this is to establish unique ownership of resources

		VkRenderPass getSwapChainRenderPass() const {
			
			return swapChain->getRenderPass();
		}

		bool isFrameUnProgress() { return isFrameStarted; }
		
		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress.");
			return commandBuffers[currentFrameIndex];
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		int getFrameIndex() const
		{
			assert(isFrameStarted && "Cannot get frame index when frame is not in progress.");
			return currentFrameIndex;
		}

	private:



		void freeCommandBuffers();
		void createCommandBuffers();
		void recreateSwapChain();



		VTAWindow& window;
		VTADevice& device;
		std::unique_ptr<VTASwapChain> swapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted{ false };
	};
}