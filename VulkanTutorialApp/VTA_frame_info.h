#pragma once
#include "VTA_camera.h"

//lib
#include <vulkan/vulkan.h>

namespace VTA {

#define MAX_LIGHTS 10

	struct PointLight
	{
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
	};

	struct GlobalUbo
	{
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		//glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f }); // light direction in world space
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, 0.02f };
		PointLight pointLightS[MAX_LIGHTS];
		int numLights;
	};


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