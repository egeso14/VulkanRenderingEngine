#pragma once
#include "VTA_device.hpp"
#include "VTA_descriptors.h"
#include <unordered_map>
#include <vector>


namespace VTA
{

    enum ResourceType {
        Buffer,
        Image
    };

    class Resource
    {
    private:
        void* objectRef;
        ResourceType resourceType;

    public:
        Resource(void* resourceObject, ResourceType type);
        VkDescriptorImageInfo GetImageDescriptorInfo();
        VkDescriptorBufferInfo GetBufferDescriptorInfo();
        ResourceType GetResourceType();

    };

    class ResourceSet
    {
    private:
        std::vector<Resource*> resources;
        std::vector<VkDescriptorSet> descriptorSet;
        std::string layoutName;
        bool isLoaded;
    public:
        ResourceSet(std::string layoutName, int framesInFlight);
        void AddResource(Resource*);
        void AssignDescriptorSet(VkDescriptorSet, int);
        std::string GetLayoutName();
        const std::vector<Resource*>& GetResources();
        VkDescriptorSet GetSetForFrame(int frame);
    };

    class ResourceLoader
    {
    public:
        ResourceLoader(
            int framesInFlight, VTADevice& device);
        void LoadResourceSet(ResourceSet& resourceSet);
        void UpdateResourceSet(ResourceSet&, int);
        void RegisterLayout(std::string layoutName, std::shared_ptr<VTADescriptorSetLayout> layout);
        void InitializeWriters();
    private:
        std::vector<VTADescriptorAllocatorGrowable> descriptorAllocators;
        std::unordered_map<std::string, VTADescriptorWriter*> writers;
        std::unordered_map<std::string, std::shared_ptr<VTADescriptorSetLayout>> layouts;
        void UpdateResourceSet(ResourceSet&);
        int framesInFlight;
        VTADevice& device;
    };
}