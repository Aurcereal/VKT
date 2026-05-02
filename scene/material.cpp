#include "material.h"
#include <iostream>
#include <variant>
#include "pipeline.h"

using namespace std;

using WDescriptorSet = std::variant<vk::DescriptorImageInfo, vk::DescriptorBufferInfo>;

// TODO: should be WShaderPipeline
void Material::Create(WPipeline* pipeline, const VulkanReferences& ref, const vector<ShaderParameter::MParameter>& parameters, int duplicationCount) {
    this->pipeline = pipeline;
    this->duplicationCount = duplicationCount;
    this->params = parameters;

    // Will probably need to do other stuff later too
    CreateDescriptorSets(ref, parameters);
}

void Material::CreateDescriptorSets(const VulkanReferences& ref, const vector<ShaderParameter::MParameter>& parameters) {
    // Need to copy the layouts to make the descriptors
    vector<vk::DescriptorSetLayout> layouts(duplicationCount, *pipeline->descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocateInfo = {
        .descriptorPool = ref.descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    // Allocate
    descriptorSets.clear();
    descriptorSets = ref.device.allocateDescriptorSets(allocateInfo);
    assert(descriptorSets.size() == duplicationCount);

    vector<void*> mallocData;
    vk::DescriptorImageInfo* aPtr;

    // Configure descriptor sets
    for (size_t i = 0; i < layouts.size(); i++) {
        vector<WDescriptorSet> ss;
        ss.reserve(parameters.size()*12); // IMPORTANT since we have pointers to vectors, 12 since texture array gives multiple descriptors
        vector<vk::WriteDescriptorSet> ssWriters;
        for (uint j = 0; j < parameters.size(); j++) {
            auto& param = parameters[j];

            // std::cout << "\t" << j << ":\t" << (int)param.type << std::endl;

            uint bufIndex;
            switch (param.type) {
            case ShaderParameter::Type::UNIFORM:
                bufIndex = i;
                if (param.uniform.uniformBuffers->size() < duplicationCount) {
                    std::cerr << "Uniform Buffer Duplication Count is SMALLER than material's duplication count, re-using final UBO for all remaining duplications!!" << std::endl;
                    bufIndex = std::min(i, param.uniform.uniformBuffers->size() - 1);
                }
                if (param.uniform.uniformBuffers->size() > duplicationCount) {
                    std::cerr << "Uniform Buffer Duplication Count is greater than material's duplication count, this is weird but COULD BE valid." << std::endl;
                }
                ss.push_back(
                    vk::DescriptorBufferInfo{
                        .buffer = (*param.uniform.uniformBuffers)[bufIndex].buffer,
                        .offset = 0,
                        .range = (*(param.uniform.uniformBuffers))[bufIndex].bufferSize
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

            case ShaderParameter::Type::DYNAMIC_UNIFORM:
                bufIndex = i;
                if (param.dynamicUniform.buffers->size() < duplicationCount) {
                    std::cerr << "Uniform Buffer Duplication Count is SMALLER than material's duplication count, re-using final UBO for all remaining duplications!!" << std::endl;
                    bufIndex = std::min(i, param.uniform.uniformBuffers->size() - 1);
                }
                if (param.dynamicUniform.buffers->size() > duplicationCount) {
                    std::cerr << "Uniform Buffer Duplication Count is greater than material's duplication count, this is weird but COULD BE valid." << std::endl;
                }
                ss.push_back(
                    vk::DescriptorBufferInfo{
                        .buffer = (*param.dynamicUniform.buffers)[bufIndex].buffer,
                        .offset = 0,
                        .range = param.dynamicUniform.singleObjectSize
                    }
                );
                ssWriters.push_back({
                    .dstSet = descriptorSets[i],
                    .dstBinding = j,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBufferDynamic,
                    .pBufferInfo = &std::get<vk::DescriptorBufferInfo>(ss[ss.size() - 1])
                    });
                break;

            case ShaderParameter::Type::COMBINED_SAMPLER:
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
            case ShaderParameter::Type::COMBINED_SAMPLER_ARRAY:
                mallocData.push_back(malloc(param.samplers.textures->size() * sizeof(vk::DescriptorImageInfo)));
                aPtr = static_cast<vk::DescriptorImageInfo*>(*(--mallocData.end()));
                for (int texInd = 0; texInd < param.samplers.textures->size(); texInd++) {
                    aPtr[texInd] = {
                         vk::DescriptorImageInfo{
                            .sampler = (*param.samplers.textures)[texInd].GetSampler(),
                            .imageView = (*param.samplers.textures)[texInd].view,
                            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal // ASSUMING READONLY TEXTURE TODO: MAYBE CHANGE
                    }};
                }
                
                ssWriters.push_back({
                    .dstSet = descriptorSets[i],
                    .dstBinding = j,
                    .dstArrayElement = 0,
                    .descriptorCount = static_cast<uint32_t>(param.samplers.textures->size()),
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = aPtr
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

            case ShaderParameter::Type::STORAGE_TEXTURE:
                ss.push_back(
                    vk::DescriptorImageInfo{
                        .imageView = param.storageTexture.texture->view,
                        .imageLayout = vk::ImageLayout::eGeneral
                    }
                );
                ssWriters.push_back({
                    .dstSet = descriptorSets[i],
                    .dstBinding = j,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageImage,
                    .pImageInfo = &std::get<vk::DescriptorImageInfo>(ss[ss.size() - 1])
                });
                break;
            }
        }

        ref.device.updateDescriptorSets(ssWriters, {});
    }

    for (void* data : mallocData) {
        free(data);
    }
}