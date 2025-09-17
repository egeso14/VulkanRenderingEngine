#pragma once

#include "VTA_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>
#include <span>

namespace VTA {

    class VTADescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VTADevice& device) : device{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<VTADescriptorSetLayout> build() const;

        private:
            VTADevice& device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        VTADescriptorSetLayout(
            VTADevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VTADescriptorSetLayout();
        VTADescriptorSetLayout(const VTADescriptorSetLayout&) = delete;
        VTADescriptorSetLayout& operator=(const VTADescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        VTADevice& device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class VTADescriptorWriter;
    };
    
    
    struct VTADescriptorAllocatorGrowable {
    public:
        struct PoolSizeRatio {
            VkDescriptorType type;
            float ratio;
        };

        void init(VkDevice device, uint32_t initialSets, std::span<PoolSizeRatio> poolRatios);
        void clear_pools(VkDevice device);
        void destroy_pools(VkDevice device);

        VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext = nullptr);
    private:
        VkDescriptorPool get_pool(VkDevice device);
        VkDescriptorPool create_pool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios);

        std::vector<PoolSizeRatio> ratios;
        std::vector<VkDescriptorPool> fullPools;
        std::vector<VkDescriptorPool> readyPools;
        uint32_t setsPerPool;

    };



    class VTADescriptorWriter {
    public:
        VTADescriptorWriter(VTADescriptorSetLayout& setLayout);

        VTADescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VTADescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        void overwrite(VkDescriptorSet& set, VTADevice& device);

    private:
        VTADescriptorSetLayout& setLayout;
        std::vector<VkWriteDescriptorSet> writes;
    };

}