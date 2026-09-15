#pragma once

#include "defines.h"
#include "buffer.h"
#include "shader-parameter.h"

using namespace std;

// Doesn't do much for the notion of re-using descriptors
class UniformBuffer {
public:
	void Create(const VulkanReferences&, vk::DeviceSize size, bool frameDuplicated);
	void SetData(int i, void*);

	ShaderParameter::SParameter GetSParameter(vk::ShaderStageFlagBits);
	ShaderParameter::MParameter GetMParameter();

	bool created = false;
private:
	vector<WBuffer> buffers;
	vk::DeviceSize byteSize;
	bool frameDuplicated;

	const VulkanReferences* pRef;
};