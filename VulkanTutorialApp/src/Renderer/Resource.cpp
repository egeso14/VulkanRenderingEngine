#include "Resource.h"
#include "VTA_image.h"
#include "VTA_Buffer.h"

VTA::Resource::Resource(void* resourceObject, ResourceType type)
{
	this->resourceType = type;
	this->objectRef = resourceObject;
}

VkDescriptorImageInfo VTA::Resource::GetImageDescriptorInfo()
{
	return static_cast<VTA_Image::Texture*>(objectRef)->descriptorInfo();
}

VkDescriptorBufferInfo VTA::Resource::GetBufferDescriptorInfo()
{
	return static_cast<VTABuffer*>(objectRef)->descriptorInfo();
}

VTA::ResourceType VTA::Resource::GetResourceType()
{
	return resourceType;
}

VTA::ResourceSet::ResourceSet(std::string layoutName, int framesInflight)
{
	this->layoutName = layoutName;
	descriptorSet.resize(framesInflight);
}

void VTA::ResourceSet::AddResource(VTA::Resource* resource)
{
	resources.push_back(resource);
}

void VTA::ResourceSet::AssignDescriptorSet(VkDescriptorSet newSet, int index)
{
	descriptorSet[index] = newSet;
}

std::string VTA::ResourceSet::GetLayoutName()
{
	return layoutName;
}

const std::vector<VTA::Resource*>& VTA::ResourceSet::GetResources()
{
	return resources;
}

VkDescriptorSet VTA::ResourceSet::GetSetForFrame(int frame)
{
	return descriptorSet[frame];
}

VTA::ResourceLoader::ResourceLoader(int framesInFlight, VTADevice& device)
	: device(device), framesInFlight(framesInFlight)
{
	//this->framesInFlight = framesInFlight;
	descriptorAllocators = std::vector<VTADescriptorAllocatorGrowable>(framesInFlight);
	for (int i = 0; i < descriptorAllocators.size(); i++)
	{
		std::vector < VTADescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};


		descriptorAllocators[i] = VTADescriptorAllocatorGrowable{};
		descriptorAllocators[i].init(device.device(), 1000, frame_sizes);
		
	}

}

void VTA::ResourceLoader::LoadResourceSet(ResourceSet& resourceSet)
{
	std::string layoutName = resourceSet.GetLayoutName();
	std::shared_ptr<VTADescriptorSetLayout> layout = layouts.find(layoutName)->second;
	auto writer = writers.find(layoutName)->second;
	auto resources = resourceSet.GetResources();

	for (int i = 0; i < framesInFlight ; i++)
	{
		VkDescriptorSet set =
			descriptorAllocators[i].allocate(
				device.device(), layout->getDescriptorSetLayout());
		
		for (int j = 0; j < resources.size(); j++)
		{
			auto resource = resources[j];
			if (resource->GetResourceType() == VTA::ResourceType::Image)
			{
				auto descriptorInfo = resource->GetImageDescriptorInfo();
				writer->writeImage(j, &descriptorInfo);
			}
			else
			{
				auto descriptorInfo = resource->GetBufferDescriptorInfo();
				writer->writeBuffer(j, &descriptorInfo);
			}
		}
		writer->overwrite(set, device);
		resourceSet.AssignDescriptorSet(set, i);
		writer->clear();
		
	}

}

void VTA::ResourceLoader::UpdateResourceSet(ResourceSet&, int)
{
}

void VTA::ResourceLoader::RegisterLayout(std::string layoutName, std::shared_ptr<VTADescriptorSetLayout> layout)
{
	layouts[layoutName] = layout;
}

void VTA::ResourceLoader::InitializeWriters()
{
	for (auto it = layouts.begin(); it != layouts.end(); ++it)
	{
		std::shared_ptr<VTADescriptorSetLayout> layout = it->second;
		writers[it->first] = new VTADescriptorWriter(*layout);
	}
}

void VTA::ResourceLoader::UpdateResourceSet(ResourceSet& resourceSet)
{
	/*std::string layoutName = resourceSet.GetLayoutName();
	std::shared_ptr<VTADescriptorSetLayout> layout = layouts.find(layoutName)->second;
	auto writer = writers.find(layoutName)->second;
	auto resources = resourceSet.GetResources();

	for (int i = 0; i < framesInFlight; i++)
	{
			

		for (int i = 0; i < resources.size(); i++)
		{
			auto resource = resources[i];
			if (resource->GetResourceType() == VTA::ResourceType::Image)
			{
				writer->writeImage(i, &resource->GetImageDescriptorInfo());
			}
		}
		writer->overwrite(set, device);
		writer->clear();

	} */
}
