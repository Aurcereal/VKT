#include "material.h"
#include <iostream>
#include <variant>

using namespace std;

//struct WDescriptorSet {
//    union {
//        vk::DescriptorImageInfo imageInfo;
//        vk::DescriptorBufferInfo bufferInfo;
//    };
//};

using WDescriptorSet = std::variant<vk::DescriptorImageInfo, vk::DescriptorBufferInfo>;

void Material::Create(const ShaderPipeline* pipeline, const vk::DescriptorPool& descriptorPool, const VulkanReferences& ref, vector<ShaderParameter::MParameter>& parameters) {
    this->pipeline = pipeline;

    // Will probably need to do other stuff later too
    CreateDescriptorSets(descriptorPool, ref, parameters);
}

void Material::CreateDescriptorSets(const vk::DescriptorPool& descriptorPool, const VulkanReferences& ref, vector<ShaderParameter::MParameter>& parameters) {
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
        vector<WDescriptorSet> ss;
        ss.reserve(parameters.size()*3);
        vector<vk::WriteDescriptorSet> ssWriters;
        for (uint j = 0; j < parameters.size(); j++) {
            auto& param = parameters[j];

            std::cout << "\t" << j << ":\t" << (int)param.type << std::endl;

            switch (param.type) {
            case ShaderParameter::Type::UNIFORM:
                ss.push_back(
                    vk::DescriptorBufferInfo{
                        .buffer = (*(param.uniform.uniformBuffers))[i].buffer,
                        .offset = 0,
                        .range = (*(param.uniform.uniformBuffers))[i].bufferSize
                    }
                );
                ssWriters.push_back({
                    .dstSet = descriptorSets[i],
                    .dstBinding = j,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo = &std::get<vk::DescriptorBufferInfo>(ss[ss.size() - 1])
                });
                break;
            case ShaderParameter::Type::SAMPLER:
                ss.push_back({
                     vk::DescriptorImageInfo{
                        .sampler = param.sampler.texture->GetSampler(),
                        .imageView = param.sampler.texture->view,
                        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal // ASSUMING READONLY TEXTURE TODO: MAYBE CHANGE
                    }
                });
                ssWriters.push_back({
                    .dstSet = descriptorSets[i],
                    .dstBinding = j,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &std::get<vk::DescriptorImageInfo>(ss[ss.size() - 1])
                });
                break;
            case ShaderParameter::Type::BUFFER:
                ss.push_back(
                    vk::DescriptorBufferInfo{
                        .buffer = param.buffer.buffer->buffer,
                        .offset = 0,
                        .range = param.buffer.buffer->bufferSize
                    }
                );
                ssWriters.push_back({
                    .dstSet = descriptorSets[i],
                    .dstBinding = j,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .pBufferInfo = &std::get<vk::DescriptorBufferInfo>(ss[ss.size() - 1])
                });
                break;
            }
        }

        ref.device.updateDescriptorSets(ssWriters, {});
    }
}