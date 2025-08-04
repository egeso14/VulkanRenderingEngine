#pragma once
#include "VTA_device.hpp"
#include "VTA_buffer.h"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>

// std
#include <memory>
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
			glm::vec3 normal;
			glm::vec2 uv;


			bool operator==(const Vertex& other) const
			{
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		struct Builder
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filePath);
		};

		VTAModel(VTADevice &device, const VTAModel::Builder &builder);
		~VTAModel();

		VTAModel(const VTAModel&) = delete;
		VTAModel& operator=(const VTAModel&) = delete; // this is to establish unique ownership of resources



		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		static std::unique_ptr<VTAModel> createModelFromFile(VTADevice& device, const std::string& filePath);

	private:
		VTADevice& device;
		
		//VkBuffer vertexBuffer; // two seperate objects. This gives control of memory management to us
		//VkDeviceMemory vertexBufferMemory;
		std::unique_ptr<VTABuffer> vertexBuffer; // using VTABuffer for better memory management
		uint32_t vertexCount;

		//VkBuffer indexBuffer; // two seperate objects. This gives control of memory management to us
		//VkDeviceMemory indexBufferMemory;
		std::unique_ptr<VTABuffer> indexBuffer; // using VTABuffer for better memory management
		uint32_t indexCount;

		bool hasIndexBuffer = false;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);
	};
}