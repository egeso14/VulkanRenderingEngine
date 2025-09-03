#include "FlatMesh.h"
#include "VTA_utils.h"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

//std
#include <cassert>
#include <iostream>
#include <unordered_map>


namespace std
{
	template<>
	struct hash<VTA_UI::FlatMesh::Vertex> {
		size_t operator() (VTA_UI::FlatMesh::Vertex const& vertex) const {
			size_t seed = 0;
			VTA::hashCombine(seed, vertex.position, vertex.uv);
			return seed;
		}
	};
}

namespace VTA_UI
{

	FlatMesh::FlatMesh(VTA::VTADevice& device, const FlatMesh::Builder& builder) : device{ device }
	{
		createVertexBuffers(builder.vertices);
		vertexCount = static_cast<uint32_t>(builder.vertices.size());
	}



	FlatMesh::~FlatMesh()
	{

	}

	void FlatMesh::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		VTA::VTABuffer stagingBuffer{ device, vertexSize, vertexCount,
								VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<VTA::VTABuffer>(device, vertexSize, vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device.copyBuffer(stagingBuffer.getBuffer(),
			vertexBuffer->getBuffer(),
			bufferSize);

		/*VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // tell Vulkan that the buffer we are creating will only be used as the source for a memory transfer operation
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // the first flag tels Vulkan that the allocated memory is accesible from the CPU
			stagingBuffer, // the second flag tells Vulkan to make sure the memory in host of device are consistent with each other
			stagingBufferMemory);*/

			/*void* data;

			vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data); // creates a region of host memory mapped to device memory and sets the beginning of data to point to the beginning of the mapped memory range
			memcpy(data, vertices.data(), static_cast<size_t>(bufferSize)); // copy the vertex data to the mapped memory
			vkUnmapMemory(device.device(), stagingBufferMemory); // unmap the memory so that it can be used by the GPU
			*/

			/*device.createBuffer(
				bufferSize,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				vertexBuffer,
				vertexBufferMemory);

			device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

			vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
			vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
			*/
	}


	void FlatMesh::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}


	void FlatMesh::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets); // bind the vertex buffer to the command buffer



		assert(vertexCount > 0 && "Cannot draw model with no vertices!");
	}

	std::vector<VkVertexInputBindingDescription> FlatMesh::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0; // binding number
		bindingDescriptions[0].stride = sizeof(Vertex); // size of the vertex structure
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // how to advance the data pointer. Vertex means that the data is per vertex, instance means that the data is per instance
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> FlatMesh::Vertex::getAttributeDescriptions()
	{

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });


		/*attributeDescriptions[0].binding = 0; // binding number
		attributeDescriptions[0].location = 0; // location number
		attributeDescriptions[0].format = ; // format of the data
		attributeDescriptions[0].offset = ; // offset of the data in the vertex structure

		attributeDescriptions[1].binding = 0; // binding number
		attributeDescriptions[1].location = 1; // location number
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // format of the data
		attributeDescriptions[1].offset = offsetof(Vertex, color); // offset of the data in the vertex structure*/

		return attributeDescriptions;
	}

	void FlatMesh::Builder::makeSimpleMesh(FlatMesh::E_MeshShapes shape, float width, float height)
	{
	}
	void FlatMesh::Builder::makeTextMesh(const char* utf8, const FontAtlas& A)
	{

		// Re-init font for kerning (you could keep it around instead).
		stbtt_fontinfo font;
		stbtt_InitFont(&font, nullptr, 0); // <-- store fontData somewhere to use here
		// (For brevity, this snippet omits passing fontData. In real code keep fontData.)

		float x = 0;
		uint32_t prev = 0;

		for (const unsigned char* s = (const unsigned char*)utf8; *s; ++s) {
			uint32_t cp = *s; // NOTE: UTF-8 decode if you need non-ASCII
			auto it = A.glyphs.find(cp);
			if (it == A.glyphs.end()) continue; // skip missing

			// Kerning (needs valid stbtt_fontinfo + scale)
			// int kern = stbtt_GetCodepointKernAdvance(&font, prev, cp);
			// x += kern * stbtt_ScaleForPixelHeight(&font, A.pixelHeight);
			prev = cp;

			const Glyph& g = it->second;

			// stb's y is down; baselineY is where text sits (top-down coords)
			float x0 = x + g.xOff;
			float y0 = 0 + g.yOff;   // yOff is typically negative
			float x1 = x0 + g.w;
			float y1 = y0 + g.h;

			// 2 triangles
			vertices.push_back({ {x0, y0, 0}, {g.u0, g.v0} });
			vertices.push_back({ {x1, y0, 0}, {g.u1, g.v0 } });
			vertices.push_back({ {x1, y1, 0}, {g.u1, g.v1} });
			vertices.push_back({ {x0, y0, 0}, {g.u0, g.v0} });
			vertices.push_back({ {x1, y1, 0}, {g.u1, g.v1} });
			vertices.push_back({ {x0, y1, 0}, {g.u0, g.v1}  });

			x += g.xAdvance; // move pen
		}
	
	}
}