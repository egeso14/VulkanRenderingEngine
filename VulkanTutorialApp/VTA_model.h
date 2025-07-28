#pragma once
#include "VTA_device.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>

// std
#include <vector>

namespace VTA
{
	class VTAModel
	{
	public:


		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		VTAModel(VTADevice &device, const std::vector<Vertex>& vertices);
		~VTAModel();

		VTAModel(const VTAModel&) = delete;
		VTAModel& operator=(const VTAModel&) = delete; // this is to establish unique ownership of resources

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		VTADevice& device;
		VkBuffer vertexBuffer; // two seperate objects. This gives control of memory management to us
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
	};
}