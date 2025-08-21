#pragma once

#include <stb_image.h>
#include "VTA_device.hpp"



namespace VTA_Image
{


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
		
		VkImageCreateInfo constructImageCreateInfo();

	};
}