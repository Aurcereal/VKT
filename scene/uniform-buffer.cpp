#include "uniform-buffer.h"

void UniformBuffer::Create(const VulkanReferences& ref, vk::DeviceSize byteSize, bool frameDuplicated) {
	this->pRef = &ref;
	this->byteSize = byteSize;
	this->frameDuplicated = frameDuplicated;
	this->created = true;

	// Create uniform buffers
	int count = frameDuplicated ? MAX_FRAMES_IN_FLIGHT : 1;
	for (int i = 0; i < count; i++) {
		buffers.emplace_back();
		buffers.back().Create(*pRef, byteSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		buffers.back().MapMemory();
	}
}

void UniformBuffer::SetData(int i, void* data) {
	// Bounds check
	int count = frameDuplicated ? MAX_FRAMES_IN_FLIGHT : 1;
	assert(i < count);

	memcpy(buffers[i].mappedMemory, data, byteSize);
}

ShaderParameter::SParameter UniformBuffer::GetSParameter(vk::ShaderStageFlagBits visibility) {
	return ShaderParameter::SParameter{ .type = ShaderParameter::Type::UNIFORM, .visibility = visibility };
}

ShaderParameter::MParameter UniformBuffer::GetMParameter() {
	return ShaderParameter::MParameter(ShaderParameter::UUniform{ .uniformBuffers = &buffers });
}

