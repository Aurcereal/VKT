#pragma once

#include "defines.h"
#include "pipeline.h"

#include "scene/buffer.h"
#include "scene/texture.h"
#include "scene/shader-parameter.h"

class WPipeline;

using namespace glm;

class Material {
public:
	void Create(const WPipeline*, const VulkanReferences&, const vector<ShaderParameter::MParameter>& parameters, int duplicationCount = MAX_FRAMES_IN_FLIGHT);// WTexture&, WTexture&, WTexture&, WTexture&, array<WBuffer*, 2>& meshBuffers);
	vector<vk::raii::DescriptorSet> descriptorSets;
private:
	void CreateDescriptorSets(const VulkanReferences&, const vector<ShaderParameter::MParameter>& parameters);// WTexture&, WTexture&, WTexture&, WTexture&, array<WBuffer*, 2>& meshBuffers);

	const WPipeline* pipeline;
	int duplicationCount;
};

struct UniformBufferObject {
	alignas(4) float off;
	alignas(16) mat4 model;
	alignas(16) mat4 view;
	alignas(16) mat4 proj;
};