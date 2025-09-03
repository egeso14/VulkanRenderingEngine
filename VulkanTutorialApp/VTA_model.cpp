#include "VTA_model.h"
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
	struct hash<VTA::VTAModel::Vertex> {
		size_t operator() (VTA::VTAModel::Vertex const& vertex) const {
			size_t seed = 0;
			VTA::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed; 
		}
	};
}

namespace VTA
{

	VTAModel::VTAModel(VTADevice& device, const VTAModel::Builder& builder) : device{ device }
	{
		createVertexBuffers(builder.vertices);
		vertexCount = static_cast<uint32_t>(builder.vertices.size());
		createIndexBuffers(builder.indices);
	}



	VTAModel::~VTAModel()
	{

	}

	void VTAModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		VTABuffer stagingBuffer{ device, vertexSize, vertexCount,
								VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<VTABuffer>(device, vertexSize, vertexCount,
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

	void VTAModel::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return; // no index buffer to create
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		VTABuffer stagingBuffer{ device, indexSize, indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<VTABuffer>(device, indexSize, indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device.copyBuffer(stagingBuffer.getBuffer(),
			indexBuffer->getBuffer(),
			bufferSize);

		/*VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // tell Vulkan that the buffer we are creating will only be used as the source for a memory transfer operation
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // the first flag tels Vulkan that the allocated memory is accesible from the CPU
			stagingBuffer, // the second flag tells Vulkan to make sure the memory in host of device are consistent with each other
			stagingBufferMemory);

		void* data;

		vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data); // creates a region of host memory mapped to device memory and sets the beginning of data to point to the beginning of the mapped memory range
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize)); // copy the vertex data to the mapped memory
		vkUnmapMemory(device.device(), stagingBufferMemory); // unmap the memory so that it can be used by the GPU

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory);

		device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);*/
	}

	void VTAModel::draw(VkCommandBuffer commandBuffer)
	{
		if (hasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0); // draw indexed model
		}
		else 
		{
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}

		
	}

	std::unique_ptr<VTAModel> VTAModel::createModelFromFile(VTADevice& device, const std::string& filePath)
	{
		Builder builder{};
		builder.loadModel(filePath); // load the model data from the file

		std::cout << "Vertex count: " << builder.vertices.size() << "\n";

		return std::make_unique<VTAModel>(device, builder); // create a new model from the loaded data
	}

	void VTAModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets); // bind the vertex buffer to the command buffer

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32); // bind the index buffer to the command buffer
		}

		assert(vertexCount > 0 && "Cannot draw model with no vertices!");
	}

	std::vector<VkVertexInputBindingDescription> VTAModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0; // binding number
		bindingDescriptions[0].stride = sizeof(Vertex); // size of the vertex structure
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // how to advance the data pointer. Vertex means that the data is per vertex, instance means that the data is per instance
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> VTAModel::Vertex::getAttributeDescriptions ()
	{
	
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
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

	void VTAModel::Builder::loadModel(const std::string& filePath)
	{
		tinyobj::attrib_t attrib; //position, color, normal, texture
		std::vector<tinyobj::shape_t>shapes; //index values for each face element
		std::vector<tinyobj::material_t>materials; // future tutorial
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
					
				}

				if (index.normal_index >= 0)
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0)
				{
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) // if the vertex is not already in the map
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size()); // add the vertex to the map with its index
					vertices.push_back(vertex); // add the vertex to the vector of vertices
				}
				indices.push_back(uniqueVertices[vertex]); // add the index of the vertex to the vector of indices
			}
		}
	}
	void VTAModel::Builder::loadTextModel(const char* utf8, const FontAtlas& A, std::vector<Vertex>& out)
	{
		std::string s = "hello ?";     // UTF-8 bytes
		const char* utf8 = s.c_str();

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
			out.push_back({  });
			out.push_back({ x1, y0, g.u1, g.v0 });
			out.push_back({ x1, y1, g.u1, g.v1 });
			out.push_back({ x0, y0, g.u0, g.v0 });
			out.push_back({ x1, y1, g.u1, g.v1 });
			out.push_back({ x0, y1, g.u0, g.v1 });

			x += g.xAdvance; // move pen
		}
		return x - startX;
	}
}