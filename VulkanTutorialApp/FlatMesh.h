#pragma once
#include "VTA_device.hpp"
#include "VTA_buffer.h"
#include "FontAtlas.h"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan expects depth values to be in the range [0, 1]
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace VTA_UI
{

	class FlatMesh
	{
	public:

		enum E_MeshShapes
		{
			Square,
			Triangle,
			Circle,
			Rectangle
		};

		struct Vertex
		{
			glm::vec3 position;
			glm::vec2 uv;


			bool operator==(const Vertex& other) const
			{
				return position == other.position && uv == other.uv;
			}

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		struct Builder
		{
			std::vector<Vertex> vertices{};
			

			void makeSimpleMesh(E_MeshShapes shape, float width, float height);
			void makeTextMesh(const char* utf8, const FontAtlas& A);
		};

		FlatMesh(VTA::VTADevice& device, const FlatMesh::Builder& builder);
		~FlatMesh();

		FlatMesh(const FlatMesh&) = delete;
		FlatMesh& operator=(const FlatMesh&) = delete; // this is to establish unique ownership of resources



		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);


	private:
		VTA::VTADevice& device;

		//VkBuffer vertexBuffer; // two seperate objects. This gives control of memory management to us
		//VkDeviceMemory vertexBufferMemory;
		std::unique_ptr<VTA::VTABuffer> vertexBuffer; // using VTABuffer for better memory management
		uint32_t vertexCount;

		
		void createVertexBuffers(const std::vector<Vertex>& vertices);
	};
	
}
