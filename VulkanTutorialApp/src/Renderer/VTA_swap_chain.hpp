#pragma once

#include "VTA_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace VTA {

class VTASwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

  VTASwapChain(VTADevice &deviceRef, VkExtent2D windowExtent);
  VTASwapChain(VTADevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<VTASwapChain> previous);

  ~VTASwapChain();

  VTASwapChain(const VTASwapChain &) = delete;
  VTASwapChain& operator=(const VTASwapChain &) = delete;

  VkFramebuffer getPass1FrameBuffer(int index) { return renderPass1FrameBuffers[index]; }
  VkFramebuffer getPass2FrameBuffer(int index) { return renderPass2FrameBuffers[index]; }
  VkRenderPass getRenderPass1() { return renderPass1; }
  VkRenderPass getRenderPass2() { return renderPass2; }
  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  size_t imageCount() { return swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }
  

  int getFrameIndex() { return currentFrame; }
  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const std::vector<VkCommandBuffer> buffers, uint32_t *imageIndex);

  bool compareSwapFormats(const VTASwapChain &swapChain) const {
    return swapChainImageFormat == swapChain.swapChainImageFormat &&
           swapChaindepthFormat == swapChain.swapChaindepthFormat;
  }

 private:
  void init();
  void createSwapChain();
  void createColorResources();
  void createImageViews();
  void createDepthResources();
  void createStencilResources();
  
  void createRenderPass1();
  void createRenderPass2();
  
  void createFramebuffers();
  void createSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat;
  VkFormat swapChaindepthFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkFramebuffer> renderPass1FrameBuffers;
  VkRenderPass renderPass1;
  
  std::vector<VkFramebuffer> renderPass2FrameBuffers;
  VkRenderPass renderPass2;

  // we need one of these since they will always be written to serially by the gpu
  VkImage colorImage;
  VkDeviceMemory colorImageMemory;
  VkImageView colorImageView;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  // we have to have multiple of these so that we aren't bottlenecked by our monitor's refresh rate
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  

  VTADevice &device;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<VTASwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};

}  // namespace lve
