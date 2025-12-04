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

		VkRenderPass getSwapChainRenderPass1() const {
			
			return swapChain->getRenderPass1();
		}

		VkRenderPass getSwapChainRenderPass2() const {

			return swapChain->getRenderPass2();
		}

		float getAspectRatio() const { return swapChain->extentAspectRatio(); }

		bool isFrameUnProgress() { return isFrameStarted; }
		
		static VkCommandBuffer getCurrentGameCommandBuffer() {
			//assert(isFrameStarted && "Cannot get command buffer when frame is not in progress.");
			return gameCommandBuffers[swapChain->getFrameIndex()];
		}

		static VkCommandBuffer getCurrentEditorUICommandBuffer() {
			//assert(isFrameStarted && "Cannot get command buffer when frame is not in progress.");
			return gameCommandBuffers[swapChain->getFrameIndex()];
		}

		static float getScreenWidth() { return swapChain->width(); }
		static float getScreenHeight() { return swapChain->height(); };
		static VkRenderPass getGUIRenderPass() { return swapChain->getRenderPass2(); };
		static VkFramebuffer getCurrentFrameBufferPass2() {
			return swapChain->getPass2FrameBuffer(swapChain->getFrameIndex());
		};
		static int getSwapchainImageCount() { return static_cast<int>(swapChain->imageCount()); };
		static int getFramesInFlight() { return VTASwapChain::MAX_FRAMES_IN_FLIGHT; };

		void beginMainFrame();

		VkCommandBuffer beginGameCommandBuffer();
		void endGameCommandBuffer();

		void endMainFrame();


		void createGameCommandBuffers();
		void createGUICommandBuffers();

		void beginSwapChainRenderPass1(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		
		

		static int getFrameIndex()
		{
			//assert(isFrameStarted && "Cannot get frame index when frame is not in progress.");
			return swapChain->getFrameIndex();
		}

	private:



		void freeCommandBuffers();
		void recreateSwapChain();



		VTAWindow& window;
		VTADevice& device;
		static std::unique_ptr<VTASwapChain> swapChain;
		static std::vector<VkCommandBuffer> gameCommandBuffers;
		static std::vector<VkCommandBuffer> guiCommandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted{ false };
	};
}