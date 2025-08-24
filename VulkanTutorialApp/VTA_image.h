#pragma once

#include <stb_image.h>
#include "VTA_device.hpp"



namespace VTA_Image
{

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits numSample,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VTA::VTADevice& device);

	VkImageView createImageView(VTA::VTADevice& device, VkImage image, VkFormat format, uint32_t mipLevel);
	// what does a texture need to be created

	class Texture
	{
	public:
		Texture(VTA::VTADevice& device, const char* filepath);
		~Texture();

		VkDescriptorImageInfo descriptorInfo();
	private:

		int texWidth;
		int texHeight;
		int texChannels;
		uint32_t imageSize;
		stbi_uc* pixels;
		const char* filepath;
		uint32_t mipLevels;

		VTA::VTADevice& device;

		//Resources
		
		VkImage textureImage;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler textureSampler;


		// Resource Descriptors
		VkDescriptorSetLayout layout;
		VkDescriptorSet descriptorSet;
		VkDescriptorPool descriptorPool;




		void createTextureImage();
		void writeToDevice();
		void createTextureImageView();
		void createTextureSampler();
		void generateMipmaps();
		
		VkImageCreateInfo constructImageCreateInfo();

	};
}