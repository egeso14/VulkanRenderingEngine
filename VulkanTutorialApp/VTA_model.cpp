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
		vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
		vkFreeMemory(device.device(), vertexBufferMemory, nullptr);

		if (hasIndexBuffer) {
			vkDestroyBuffer(device.device(), indexBuffer, nullptr);
			vkFreeMemory(device.device(), indexBufferMemory, nullptr);
		}
	}

	void VTAModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		
		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // tell Vulkan that the buffer we are creating will only be used as the source for a memory transfer operation
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // the first flag tels Vulkan that the allocated memory is accesible from the CPU
			stagingBuffer, // the second flag tells Vulkan to make sure the memory in host of device are consistent with each other
			stagingBufferMemory);

		void* data;

		vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data); // creates a region of host memory mapped to device memory and sets the beginning of data to point to the beginning of the mapped memory range
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize)); // copy the vertex data to the mapped memory
		vkUnmapMemory(device.device(), stagingBufferMemory); // unmap the memory so that it can be used by the GPU

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer, 
			vertexBufferMemory);

		device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
	}

	void VTAModel::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return; // no index buffer to create
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		VkBuffer stagingBuffer;
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
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
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
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets); // bind the vertex buffer to the command buffer

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32); // bind the index buffer to the command buffer
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

	std::vector<VkVertexInputAttributeDescription> VTAModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0; // binding number
		attributeDescriptions[0].location = 0; // location number
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of the data
		attributeDescriptions[0].offset = offsetof(Vertex, position); // offset of the data in the vertex structure

		attributeDescriptions[1].binding = 0; // binding number
		attributeDescriptions[1].location = 1; // location number
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // format of the data
		attributeDescriptions[1].offset = offsetof(Vertex, color); // offset of the data in the vertex structure

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

					auto colorIndex = 3 * index.vertex_index + 2;
					if (colorIndex < attrib.colors.size())
					{
						vertex.color = {
							attrib.colors[colorIndex - 2],
							attrib.colors[colorIndex - 1],
							attrib.colors[colorIndex]
						};
					}
					else
					{
						vertex.color = { 1.0f, 1.0f, 1.0f }; // default color if no color is specified
					}
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
}