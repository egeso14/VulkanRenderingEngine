#pragma once
#include "VTA_camera.h"
#include "VTA_Widget.h"

//lib
#include <vulkan/vulkan.h>

namespace VTA_UI {


	struct FrameInfo_EditorUI
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VkDescriptorSet uiDescriptorSet;
	};
}