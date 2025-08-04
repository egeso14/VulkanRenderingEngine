#pragma once

#include "VTA_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

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

    class VTADescriptorPool {
    public:
        class Builder {
        public:
            Builder(VTADevice& device) : device{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VTADescriptorPool> build() const;

        private:
            VTADevice& device;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        VTADescriptorPool(
            VTADevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VTADescriptorPool();
        VTADescriptorPool(const VTADescriptorPool&) = delete;
        VTADescriptorPool& operator=(const VTADescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        VTADevice& device;
        VkDescriptorPool descriptorPool;

        friend class VTADescriptorWriter;
    };

    class VTADescriptorWriter {
    public:
        VTADescriptorWriter(VTADescriptorSetLayout& setLayout, VTADescriptorPool& pool);

        VTADescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VTADescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VTADescriptorSetLayout& setLayout;
        VTADescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}