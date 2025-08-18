#include "VTA_descriptors.h"

// std
#include <cassert>
#include <stdexcept>

namespace VTA {

    // *************** Descriptor Set Layout Builder *********************

    VTADescriptorSetLayout::Builder& VTADescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<VTADescriptorSetLayout> VTADescriptorSetLayout::Builder::build() const {
        return std::make_unique<VTADescriptorSetLayout>(device, bindings);
    }

    // *************** Descriptor Set Layout *********************

    VTADescriptorSetLayout::VTADescriptorSetLayout(
        VTADevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : device{ device }, bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            device.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    VTADescriptorSetLayout::~VTADescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool *********************

    VkDescriptorPool VTADescriptorAllocatorGrowable::get_pool(VkDevice device)
    {
        VkDescriptorPool newPool;

        if (readyPools.size() != 0)
        {
            newPool = readyPools.back();
            readyPools.pop_back();
        }
        else {
            newPool = create_pool(device, setsPerPool, ratios);
            setsPerPool = setsPerPool * 1.5f;
            if (setsPerPool > 4092)
            {
                setsPerPool = 4092;
            }
        }
        return newPool;
    }

    VkDescriptorPool VTADescriptorAllocatorGrowable::create_pool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios)
    {
        std::vector<VkDescriptorPoolSize> poolSizes;
        for (PoolSizeRatio ratio : ratios)
        {
            poolSizes.push_back(VkDescriptorPoolSize{
                .type = ratio.type,
                .descriptorCount = uint32_t(ratio.ratio * setCount)
                });
        }

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = 0;
        pool_info.maxSets = setCount;
        pool_info.poolSizeCount = (uint32_t)poolSizes.size();
        pool_info.pPoolSizes = poolSizes.data();
		
		VkDescriptorPool newPool;
		vkCreateDescriptorPool(device, &pool_info, nullptr, &newPool);
		return newPool;

    }

    void VTADescriptorAllocatorGrowable::init(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
    {
        ratios.clear();

        for (auto r : poolRatios) {
            ratios.push_back(r);
        }

        VkDescriptorPool newPool = create_pool(device, maxSets, poolRatios);

        setsPerPool = maxSets * 1.5; //grow it next allocation

        readyPools.push_back(newPool);
    }

    void VTADescriptorAllocatorGrowable::clear_pools(VkDevice device)
    {
        for (auto p : readyPools) {
            vkResetDescriptorPool(device, p, 0);
        }
        for (auto p : fullPools) {
            vkResetDescriptorPool(device, p, 0);
            readyPools.push_back(p);
        }
        fullPools.clear();
    }

    void VTADescriptorAllocatorGrowable::destroy_pools(VkDevice device)
    {
        for (auto p : readyPools) {
            vkDestroyDescriptorPool(device, p, nullptr);
        }
        readyPools.clear();
        for (auto p : fullPools) {
            vkDestroyDescriptorPool(device, p, nullptr);
        }
        fullPools.clear();
    }

    VkDescriptorSet VTADescriptorAllocatorGrowable::allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext)
    {
        //get or create a pool to allocate from
        VkDescriptorPool poolToUse = get_pool(device);

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.pNext = pNext;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = poolToUse;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet ds;
        VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &ds);

        //allocation failed. Try again
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {

            fullPools.push_back(poolToUse);

            poolToUse = get_pool(device);
            allocInfo.descriptorPool = poolToUse;

            if (vkAllocateDescriptorSets(device, &allocInfo, &ds) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor set after growing pool!");
			}
        }

        readyPools.push_back(poolToUse);
        return ds;
    }




    // *************** Descriptor Writer *********************

    VTADescriptorWriter::VTADescriptorWriter(VTADescriptorSetLayout& setLayout)
        : setLayout{ setLayout } {
    }


    VTADescriptorWriter& VTADescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    VTADescriptorWriter& VTADescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }


    void VTADescriptorWriter::overwrite(VkDescriptorSet& set, VTADevice& device) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(device.device(), writes.size(), writes.data(), 0, nullptr);
    }

}