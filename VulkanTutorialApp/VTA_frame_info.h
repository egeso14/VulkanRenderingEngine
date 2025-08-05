#pragma once
#include "VTA_camera.h"

//lib
#include <vulkan/vulkan.h>

namespace VTA {
	struct FrameInfo
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VTACamera& camera;
		VkDescriptorSet globalDescriptorSet;
		VTAGameObject::Map& gameObjects;
	};
}