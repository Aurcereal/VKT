#include "material.h"

using namespace std;

void Material::Create(const ShaderPipeline* pipeline, const vk::DescriptorPool& descriptorPool, const VulkanReferences& ref, const vector<WBuffer>& uniformBuffers, WTexture& albedo, WTexture& metallic, WTexture& roughness, WTexture& ao, array<WBuffer*, 2>& meshBuffers) {
    this->pipeline = pipeline;

    // Will probably need to do other stuff later too
    CreateDescriptorSets(descriptorPool, ref, uniformBuffers, albedo, metallic, roughness, ao, meshBuffers);
}

void Material::CreateDescriptorSets(const vk::DescriptorPool& descriptorPool, const VulkanReferences& ref, const vector<WBuffer>& uniformBuffers, WTexture& albedo, WTexture& metallic, WTexture& roughness, WTexture& ao, array<WBuffer*, 2>& meshBuffers) {
    // Need to copy the layouts to make the descriptors
    vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *pipeline->descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocateInfo = {
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    // Allocate
    descriptorSets.clear();
    descriptorSets = ref.device.allocateDescriptorSets(allocateInfo);
    assert(descriptorSets.size() == MAX_FRAMES_IN_FLIGHT);

    // Configure descriptor sets
    for (size_t i = 0; i < layouts.size(); i++) {
        // Descriptors that use buffers are configured with DescriptorBufferInfo
        vk::DescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffers[i].buffer,
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };
        vector texturesInfo = {
            vk::DescriptorImageInfo {
                .sampler = albedo.GetSampler(),
                .imageView = albedo.view,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
            },
            vk::DescriptorImageInfo {
                .sampler = metallic.GetSampler(),
                .imageView = metallic.view,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
            },
            vk::DescriptorImageInfo {
                .sampler = roughness.GetSampler(),
                .imageView = roughness.view,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
            },
            vk::DescriptorImageInfo {
                .sampler = ao.GetSampler(),
                .imageView = ao.view,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
            },
        };
        vk::DescriptorBufferInfo vertexBufferInfo = {
            .buffer = meshBuffers[0]->buffer,
            .offset = 0,
            .range = meshBuffers[0]->bufferSize
        };
        vk::DescriptorBufferInfo indexBufferInfo = {
            .buffer = meshBuffers[1]->buffer,
            .offset = 0,
            .range = meshBuffers[1]->bufferSize
        };

        std::array descriptorWrites = {
            vk::WriteDescriptorSet {
                .dstSet = descriptorSets[i], // descriptor set to update
                .dstBinding = 0, // binding from beginning of CreateDescriptorSetLayout - the shader binding slot
                .dstArrayElement = 0, // descriptors can be arrays
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &bufferInfo // for buffer data, pImageInfo would be used for image data
            },

            vk::WriteDescriptorSet{
                .dstSet = descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &texturesInfo[0]
            },
            vk::WriteDescriptorSet{
                .dstSet = descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &texturesInfo[1]
            },
            vk::WriteDescriptorSet{
                .dstSet = descriptorSets[i],
                .dstBinding = 3,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &texturesInfo[2]
            },
            vk::WriteDescriptorSet{
                .dstSet = descriptorSets[i],
                .dstBinding = 4,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &texturesInfo[3]
            },

            vk::WriteDescriptorSet{
                .dstSet = descriptorSets[i],
                .dstBinding = 5,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eStorageBuffer,
                .pBufferInfo = &vertexBufferInfo
            },
            vk::WriteDescriptorSet{
                .dstSet = descriptorSets[i],
                .dstBinding = 6,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eStorageBuffer,
                .pBufferInfo = &indexBufferInfo
            },
        };

        ref.device.updateDescriptorSets(descriptorWrites, {});
    }
}