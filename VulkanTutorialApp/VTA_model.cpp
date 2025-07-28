#include "VTA_model.h"

//std
#include <cassert>

namespace VTA
{

	VTAModel::VTAModel(VTADevice& device, const std::vector<Vertex>& vertices) : device{ device }
	{
		createVertexBuffers(vertices);
		vertexCount = static_cast<uint32_t>(vertices.size());
	}



	VTAModel::~VTAModel()
	{
		vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
		vkFreeMemory(device.device(), vertexBufferMemory, nullptr);
	}

	void VTAModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // the first flag tels Vulkan that the allocated memory is accesible from the CPU
			vertexBuffer, // the second flag tells Vulkan to make sure the memory in host of device are consistent with each other
			vertexBufferMemory);

		void* data;

		vkMapMemory(device.device(), vertexBufferMemory, 0, bufferSize, 0, &data); // creates a region of host memory mapped to device memory and sets the beginning of data to point to the beginning of the mapped memory range
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize)); // copy the vertex data to the mapped memory
		vkUnmapMemory(device.device(), vertexBufferMemory); // unmap the memory so that it can be used by the GPU

		//my question is this: why do we have a seperate buffer object?
	}

	void VTAModel::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void VTAModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets); // bind the vertex buffer to the command buffer
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
}